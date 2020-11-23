#include "spi.h"
#include "hardware.h"
#include "Assert.h"
#include "GPIO.h"
#include "port.h"
#include "CircularBuffer.h"
#include "stdlib.h"

typedef struct
{
    SPI_BitsPerFrame_t bitsPerFrame;
    //volatile uint32_t command;	   //Data command
    //volatile uint32_t lastCommand; //Last data command

    uint8_t fifoSize; //Fifo datasize

    //volatile bool isPcsActiveAfterTransfer; //After last frame transfer
    //volatile bool isThereExtraByte;

    uint8_t *volatile txData;
    uint8_t *volatile rxData;
    volatile size_t remainingSendByteCount;
    volatile size_t remainingReceiveByteCount;
    size_t totalByteCount;

    volatile uint8_t state; //Transfer state

    SPI_onTransferCompleteCallback callback; //onComplete
    void *userData;                          //callback param
} SPI_MasterHandle_t;

/*Transfer struct.*/
typedef struct
{
    uint8_t *txData;
    uint8_t *rxData;
    volatile size_t dataSize;
} SPI_Transfer_t;

void SPI_MasterInit(SPI_Instance_t n, SPI_MasterConfig_t *config)
{
    ///////////////////////////////////////////////////////////////////////
    //*						Check if everything is fine
    ///////////////////////////////////////////////////////////////////////
    //* First check if the n passed is a possible value (<3)
    ASSERT(n < FSL_FEATURE_SOC_DSPI_COUNT);
    //* Second check if the CTAR used is a possible value (number between 0 and Number of CTAR registers (2) )
    ASSERT((config->CTARUsed) < FSL_FEATURE_DSPI_CTAR_COUNT);

    ///////////////////////////////////////////////////////////////////////
    //*		Enable clock gating and NVIC for the n SPI_Instance passed
    ///////////////////////////////////////////////////////////////////////
    if (n == SPI_0)
    {
        SIM->SCGC6 |= SIM_SCGC6_SPI0_MASK;
        NVIC_EnableIRQ(SPI0_IRQn);
    }
    else if (n == SPI_1)
    {
        SIM->SCGC6 |= SIM_SCGC6_SPI1_MASK;
        NVIC_EnableIRQ(SPI1_IRQn);
    }
    else if (n == SPI_2)
    {
        SIM->SCGC3 |= SIM_SCGC3_SPI2_MASK;
        NVIC_EnableIRQ(SPI2_IRQn);
    }
    //* Check if the module is in stop state (a register inside SPIx_SR)
    ASSERT((SPIs[n]->SR & SPI_SR_TXRXS_MASK) != SPI_SR_TXRXS_MASK);

    ///////////////////////////////////////////////////////////////////////
    //*				Clock and Transfer Attributes Register (CTAR)
    ///////////////////////////////////////////////////////////////////////

    //* Check if in the actual config the master is enabled
    if (config->enableMaster)
    {
        //* Sets the clock and transfer attributes register (CTAR ON MASTER MODE) selected on config
        SPIs[n]->CTAR[config->CTARUsed] =
            SPI_CTAR_FMSZ(config->bitsPerFrame) |
            SPI_CTAR_CPOL(config->clockConfig.clockPolarity) |
            SPI_CTAR_CPHA(config->clockConfig.clockPhase) |
            SPI_CTAR_LSBFE(config->bitOrder) |
            SPI_CTAR_PCSSCK(1) | //* This function configures the PCS to SCK delay pre-scalar
            //SPI_CTAR_CSSCK(config->chipSelectToClkDelay) | //* PCS to SCK Delay Scaler: then t_CSC = (1/fP ) x PCSSCK x CSSCK. (page 1513 ref manual)
            SPI_CTAR_PASC(1) | //* This function configures the after SCK delay delay pre-scalar
            //SPI_CTAR_ASC(config->clockDelayScaler) |	   //*After SCK Delay Scaler: tASC = (1/fP) x PASC x ASC (page 1513 ref manual)
            SPI_CTAR_PDT(3) | //*This function configures delayAfterTransferPreScale (PDT) 3 means 11 wich represent that the Delay after Transfer Prescaler value is 7.
            //SPI_CTAR_DT(config->delayAfterTransfer) |	   //*Delay After Transfer Scaler: tDT = (1/fP ) x PDT x DT
            SPI_CTAR_DBR(1) |              //* Double Baud Rate, Doubles the effective baud rate of the Serial Communications Clock
            SPI_CTAR_PBR(0) |              //* Sets the SCK Duty Cycle on 50/50
            SPI_CTAR_BR(config->baudRate); //* Baud Rate Scaler: SCK baud rate = (fP /PBR) x [(1+DBR)/BR]
    }

    ///////////////////////////////////////////////////////////////////////
    //*				   Module configuration register (MCR)
    ///////////////////////////////////////////////////////////////////////
    //*Enable the module clock (0(~SPI_MCR_MDIS_MASK) Enables the module clocks and 1 (SPI_MCR_MDIS_MASK) allows external logic to disable the module clocks.)
    SPIs[n]->MCR &= ~SPI_MCR_MDIS_MASK;
    SPIs[n]->MCR =
        SPI_MCR_MSTR(1) | //* Enables Master mode
        //SPI_MCR_CONT_SCKE(config->continuousSerialCLK) |	 //* Enables continuos clock or not
        SPI_MCR_DCONF(0) |                                   //* 0 is the only option available
        SPI_MCR_FRZ(1) |                                     //* Halt serial transfers in Debug mode.
        SPI_MCR_MTFE(0) |                                    //* Modified SPI transfer format disabled.
        SPI_MCR_PCSSE(0) |                                   //* PCS5/ PCSS is used as the Peripheral Chip Select[5] signal.
        SPI_MCR_ROOE(config->enableRxFIFOverflowOverwrite) | //* Receive FIFO Overflow Overwrite Enable (0 Incoming data is ignored. 1 Incoming data is shifted into the shift register.)
        SPI_MCR_PCSIS(config->chipSelectPolarity) |          //* Peripheral Chip Select x Inactive State (0 The inactive state of PCSx is low. 1 The inactive state of PCSx is high.)
        SPI_MCR_DOZE(0) |                                    //* Doze mode has no effect on the module
        SPI_MCR_MDIS(0) |                                    //* Enables the module clocks
        SPI_MCR_DIS_TXF(config->disableTxFIFO) |             //* 0 TX FIFO is enabled. 1 TX FIFO is disabled.
        SPI_MCR_DIS_RXF(config->disableRxFIFO) |             //* 0 RX FIFO is enabled. 1 RX FIFO is disabled.
        SPI_MCR_HALT(1);

    ///////////////////////////////////////////////////////////////////////
    //*				   Output Config
    ///////////////////////////////////////////////////////////////////////
    //gpioMode(PORTNUM2PIN(PC,5),OUTPUT);
    PORT_Config portConfig;
    PORT_GetPinDefaultConfig(&portConfig);
    PORT_PinConfig(PORT_D, 0, &portConfig, PORT_MuxAlt2); //* CS
    PORT_PinConfig(PORT_D, 1, &portConfig, PORT_MuxAlt2); //* SCK
    PORT_PinConfig(PORT_D, 2, &portConfig, PORT_MuxAlt2); //* SOUT
    PORT_PinConfig(PORT_D, 3, &portConfig, PORT_MuxAlt2); //* SIN
}