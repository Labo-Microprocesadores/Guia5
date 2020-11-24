#include "spi.h"
#include "hardware.h"
#include "Assert.h"
#include "GPIO.h"
#include "port.h"
#include "CircularBuffer.h"
#include "stdlib.h"

#define BUFFER_SIZE 100
#define TX_QUEUE_SIZE 100
#define RX_QUEUE_SIZE 100

static CircularBuffer_t txCircularBuffer;
static CircularBuffer_t rxCircularBuffer;
uint32_t transmitBuffer[TX_QUEUE_SIZE]; // Buffer for tx
uint16_t recieveBuffer[RX_QUEUE_SIZE];  // Buffer for rx

//*Creates the array of spis and sets on the default value
static SPI_Type *SPIs[] = SPI_BASE_ADDRS;

static SPI_onTransferCompleteCallback transferCallback;
// Bytes left in current transfer
static uint8_t bytesLeft;
static SPI_Instance_t currentSPIInstance;

typedef struct
{
    SPI_PCSignal_t pcsSignal;
    bool eoq;
    uint16_t message;
} SPI_Data_t;

void SPI_MasterInit(SPI_Instance_t n, SPI_MasterConfig_t *config)
{
    ///////////////////////////////////////////////////////////////////////
    //*						Check if everything is fine
    ///////////////////////////////////////////////////////////////////////
    //* First check if the n passed is a possible value (<3)
    ASSERT(n < FSL_FEATURE_SOC_DSPI_COUNT);
    //* Second check if the CTAR used is a possible value (number between 0 and Number of CTAR registers (2) )
    ASSERT((config->CTARUsed) < FSL_FEATURE_DSPI_CTAR_COUNT);

    currentSPIInstance = n; //Store SPI instance
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
            SPI_CTAR_PASC(1) |                        //* This function configures the after SCK delay delay pre-scalar
            SPI_CTAR_ASC(config->clockDelayScaler) |  //*After SCK Delay Scaler: tASC = (1/fP) x PASC x ASC (page 1513 ref manual)
            SPI_CTAR_PDT(3) |                         //*This function configures delayAfterTransferPreScale (PDT) 3 means 11 wich represent that the Delay after Transfer Prescaler value is 7.
            SPI_CTAR_DT(config->delayAfterTransfer) | //*Delay After Transfer Scaler: tDT = (1/fP ) x PDT x DT
            SPI_CTAR_DBR(1) |                         //* Double Baud Rate, Doubles the effective baud rate of the Serial Communications Clock
            SPI_CTAR_PBR(0) |                         //* Sets the SCK Duty Cycle on 50/50
            SPI_CTAR_BR(config->baudRate);            //* Baud Rate Scaler: SCK baud rate = (fP /PBR) x [(1+DBR)/BR]
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
    //*				  Interruption Config for SPI
    ///////////////////////////////////////////////////////////////////////
    //* Status Register (SPIx_SR)
    SPIs[n]->SR =
        SPI_SR_EOQF(1) |
        SPI_SR_TCF(1) |
        SPI_SR_TFUF(1) |
        SPI_SR_TFFF(1) |
        SPI_SR_RFOF(1) |
        SPI_SR_RFDF(1);
    //* DMA/Interrupt Request Select and Enable Register (SPIx_RSER)
    SPIs[n]->RSER =
        SPI_RSER_RFDF_RE(1) |
        SPI_RSER_EOQF_RE(1);
    ///////////////////////////////////////////////////////////////////////
    //*				   Output Config
    ///////////////////////////////////////////////////////////////////////
    // Clock gating of the PORT peripheral
    SIM->SCGC5 |= SIM_SCGC5_PORTD(1);

    //gpioMode(PORTNUM2PIN(PC,5),OUTPUT);
    PORT_Config portConfig;
    PORT_GetPinDefaultConfig(&portConfig);
    PORT_PinConfig(PORT_D, 0, &portConfig, PORT_MuxAlt2); //* CS
    PORT_PinConfig(PORT_D, 1, &portConfig, PORT_MuxAlt2); //* SCK
    PORT_PinConfig(PORT_D, 2, &portConfig, PORT_MuxAlt2); //* SOUT
    PORT_PinConfig(PORT_D, 3, &portConfig, PORT_MuxAlt2); //* SIN

    txCircularBuffer = newCircularBuffer(transmitBuffer, BUFFER_SIZE, sizeof(uint8_t));
    rxCircularBuffer = newCircularBuffer(recieveBuffer, BUFFER_SIZE, sizeof(uint8_t));
}

int SPI_SendFrame(uint8_t *data, uint8_t length, SPI_onTransferCompleteCallback callback)
{
    ASSERT(data != NULL);
    ASSERT(length < spaceLeft(&transmitBuffer));

    int bytesSent = 0;
    for (int i = 0; i < length; i++)
        if (push(&transmitBuffer, data + i) == false)
        {
            bytesSent = i;
            break;
        }

    if (bytesSent == 0) // If didn't break..
        bytesSent = length;

    // Store bytes left
    bytesLeft = bytesSent;

    transferCallback = callback;

    // Enable interrupts to start copying bytes from circular buffer to SPI module
    SPIs[currentSPIInstance]->MCR |= SPI_MCR_CLR_TXF_MASK;
    SPIs[currentSPIInstance]->RSER |= SPI_RSER_TFFF_RE_MASK;
    SPIs[currentSPIInstance]->RSER &= ~SPI_RSER_TFFF_DIRS_MASK;
    return bytesSent;
}

bool SPI_SendMessage(SPI_Type *base, SPI_PCSignal_t pcsSignal, const uint16_t messageToSend[], size_t messageLength, bool onlyRead)
{
    /*1. Check available space in the buffer*/
    if (numberOfElementsLeft(&txCircularBuffer) < messageLength)
        return false;

    /*2.Push the message to the circular buffer. If only read is needed -> Send any character as trash*/

    if (onlyRead)
    {
        for (int i = 0; i < messageLength; i++)
        {
            push(&txCircularBuffer, 'a'); //Trash
        }
    }
    else
    {
        int untilFifoSizeCounter = 0; //Counter that reinitiates when it equals fifoSize. Function: send an "eoq" when this happens.
        for (int i = 0; i < messageLength; i++)
        {
            bool reachedFifoSize = false; //untilFifoSizeCounter reached fifoSize.
            bool eoq = false;             //eoq flag to send.

            if (++untilFifoSizeCounter == SPIs[currentSPIInstance].hwFifoSize)
            {
                reachedFifoSize = true;
                untilFifoSizeCounter = 0; //Reinitiates counter.
            }

            if (i == messageLength - 1 || reachedFifoSize) //The message ends or untilFifoSizeCounter reached fifoSize.
                eoq = true;

            SPI_Data_t sendingData = {
                .message = messageToSend[i],
                .pcsSignal = pcsSignal,
                .eoq = eoq};

            push(&txCircularBuffer, (void *)&sendingData);
        }
    }

    /*3. Comenzar la transmision*/
    while ((SPIs[currentSPIInstance]->SR & SPI_SR_TFFF_MASK) && !isEmpty(&txCircularBuffer))
    {
        uint32_t bufferData;
        if (pop(&txCircularBuffer, &bufferData))
        {
            //TODO: Adaptarlo a nuestro circular buffer
            SPIs[currentSPIInstance]->PUSHR = bufferData;

            SPIs[currentSPIInstance]->SR = SPI_SR_TFFF_MASK;
        }
    }

    SPIs[currentSPIInstance]->MCR = (SPIs[currentSPIInstance]->MCR & ~SPI_MCR_HALT_MASK) | SPI_MCR_HALT(0);
    return true;
}
