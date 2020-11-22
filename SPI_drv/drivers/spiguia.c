#include "spiguia.h"
#include "hardware.h"
#include "Assert.h"
#include "GPIO.h"
#include "port.h"
#include "CircularBuffer.h"
#include "stdlib.h"
//#include "CPUTimeMeasurement.h"
#define BUFFER_SIZE					256

//*Creates the array of spis and sets on the default value
static SPI_Type * SPIs[] = SPI_BASE_ADDRS;


// Bytes left in current transfer
static uint8_t bytesLeft;

static SPI_Callback transferCallback;

NEW_CIRCULAR_BUFFER(transmitBuffer,BUFFER_SIZE,sizeof(uint8_t));
NEW_CIRCULAR_BUFFER(recieveBuffer,BUFFER_SIZE,sizeof(uint8_t));


void SPI_ClearTxFIFO(SPI_Instance n);
void SPI_StartCountingTxFIFO(SPI_Instance n);
void SPI_HaltModule(SPI_Instance n);
void SPI_RunModule(SPI_Instance n);
void SPI_SetEOQ(SPI_Instance n);
void SPI_ClearEOQ(SPI_Instance n);



void SPI_MasterGetDefaultConfig(SPI_MasterConfig * config)
{
//*MCR config
	config->enableRxFIFOverflowOverwrite = false;
	config->disableTxFIFO = false;
	config->disableRxFIFO = false;
	config->chipSelectActiveState = SPI_PCSActiveLow;
	config->enableMaster = true;
	config->delayAfterTransferPreScale = SPI_DelayAfterTransferPreScaleOne;
	config->delayAfterTransfer = SPI_eightPowerDelay;
	config->continuousSerialCLK = false;		//CON ESTE HABILITO EL CONTINUOUS CLK
	config->CTARUsed = SPI_CTAR_0;

//*CTAR config
	config->bitsPerFrame = SPI_eightBitsFrame;
	config->polarity = SPI_ClockActiveHigh;
	config->phase = SPI_ClockPhaseSecondEdge;
	config->direction = SPI_FirstLSB;
	config->clockDelayScaler = SPI_eightPowerDelay;
	config->baudRate = SPI_twoPowerDelay;
	config->chipSelectToClkDelay = SPI_eightPowerDelay;


//	config->enableStopInWaitMode;
//	config->outputMode;
//	config->pinMode;

}

void SPI_MasterInit(SPI_Instance n, SPI_MasterConfig * config)
{
	///////////////////////////////////////////////////////////////////////
	//*						Check if everything is fine
	///////////////////////////////////////////////////////////////////////
	//* First check if the n passed is a possible value (<3)
	ASSERT(n<FSL_FEATURE_SOC_DSPI_COUNT);
	//* Second check if the CTAR used is a possible value (number between 0 and Number of CTAR registers (2) )
	ASSERT((config->CTARUsed)<FSL_FEATURE_DSPI_CTAR_COUNT);


	///////////////////////////////////////////////////////////////////////
	//*		Enable clock gating and NVIC for the n SPI_Instance passed
	///////////////////////////////////////////////////////////////////////
	if(n==SPI_0)
	{
		SIM->SCGC6 |= SIM_SCGC6_SPI0_MASK;
		NVIC_EnableIRQ(SPI0_IRQn);
	}
	else if (n==SPI_1)
	{
		SIM->SCGC6 |= SIM_SCGC6_SPI1_MASK;
		NVIC_EnableIRQ(SPI1_IRQn);
	}
	else if (n==SPI_2)
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
		SPI_CTAR_CPOL(config->polarity) |
		SPI_CTAR_CPHA(config->phase) |
		SPI_CTAR_LSBFE(config->direction) |
		SPI_CTAR_PCSSCK(1) | 								//* This function configures the PCS to SCK delay pre-scalar
		SPI_CTAR_CSSCK(config->chipSelectToClkDelay) |  	//* PCS to SCK Delay Scaler: then t_CSC = (1/fP ) x PCSSCK x CSSCK. (page 1513 ref manual)
		SPI_CTAR_PASC(1) | 									//* This function configures the after SCK delay delay pre-scalar
		SPI_CTAR_ASC(config->clockDelayScaler) |			//*After SCK Delay Scaler: tASC = (1/fP) x PASC x ASC (page 1513 ref manual)
		SPI_CTAR_PDT(3)|									//*This function configures delayAfterTransferPreScale (PDT) 3 means 11 wich represent that the Delay after Transfer Prescaler value is 7.
		SPI_CTAR_DT(config->delayAfterTransfer) |  			//*Delay After Transfer Scaler: tDT = (1/fP ) x PDT x DT
		SPI_CTAR_DBR(1)| 									//* Double Baud Rate, Doubles the effective baud rate of the Serial Communications Clock
		SPI_CTAR_PBR(0)|									//* Sets the SCK Duty Cycle on 50/50
		SPI_CTAR_BR(config->baudRate);  					//* Baud Rate Scaler: SCK baud rate = (fP /PBR) x [(1+DBR)/BR]
	}else
	{
	//*	Clock and transfer attributes register (CTAR ON SLAVE MODE) or put all to sleep at least 
	//*	SPIs[n]->CTAR_SLAVE
	}


	///////////////////////////////////////////////////////////////////////
	//*				   Module configuration register (MCR)
	///////////////////////////////////////////////////////////////////////

	//?Why?
	//*No estan configurados: SMPL_PT 
	//? End of why (creo que porque no es necesario)

	//*Enable the module clock (0(~SPI_MCR_MDIS_MASK) Enables the module clocks and 1 (SPI_MCR_MDIS_MASK) allows external logic to disable the module clocks.)
	SPIs[n]->MCR &= ~SPI_MCR_MDIS_MASK;
	SPIs[n]->MCR =
			SPI_MCR_MSTR(1) | 									//* Enables Master mode
			SPI_MCR_CONT_SCKE(config->continuousSerialCLK) | 	//* Enables continuos clock or not
			SPI_MCR_DCONF(0) |									//* 0 is the only option available
			SPI_MCR_FRZ(1) |									//* Halt serial transfers in Debug mode.
			SPI_MCR_MTFE(0) |									//* Modified SPI transfer format disabled.
			SPI_MCR_PCSSE(0) |									//* PCS5/ PCSS is used as the Peripheral Chip Select[5] signal.
			SPI_MCR_ROOE(config->enableRxFIFOverflowOverwrite) |//* Receive FIFO Overflow Overwrite Enable (0 Incoming data is ignored. 1 Incoming data is shifted into the shift register.)
			SPI_MCR_PCSIS(config->chipSelectActiveState) |		//* Peripheral Chip Select x Inactive State (0 The inactive state of PCSx is low. 1 The inactive state of PCSx is high.)
			SPI_MCR_DOZE(0) |									//* Doze mode has no effect on the module
			SPI_MCR_MDIS(0) |									//* Enables the module clocks
			SPI_MCR_DIS_TXF(config->disableTxFIFO) |			//* 0 TX FIFO is enabled. 1 TX FIFO is disabled.
			SPI_MCR_DIS_RXF(config->disableRxFIFO) |			//* 0 RX FIFO is enabled. 1 RX FIFO is disabled.
			SPI_MCR_HALT(1);									//* 1 Stop transfers. until the program want to send something


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



void SPI_EnableTxFIFOFillDMARequests(SPI_Instance n)
{
	SPIs[n]->RSER |= SPI_RSER_TFFF_RE_MASK | SPI_RSER_TFFF_DIRS_MASK;
}
void SPI_EnableTxFIFOFillInterruptRequests(SPI_Instance n)
{
	SPIs[n]->RSER |= SPI_RSER_TFFF_RE_MASK;
	SPIs[n]->RSER &= ~SPI_RSER_TFFF_DIRS_MASK;
}
void SPI_DisableTxFIFOFillRequests(SPI_Instance n)
{
	SPIs[n]->RSER &= ~SPI_RSER_TFFF_RE_MASK;
}

void SPI_EnableEOQInterruptRequests(SPI_Instance n)
{
	SPIs[n]->RSER |= SPI_RSER_EOQF_RE_MASK;
}
void SPI_DisableEOQInterruptRequests(SPI_Instance n)
{
	SPIs[n]->RSER &= ~SPI_RSER_EOQF_RE_MASK;
}

void SPI_HaltModule(SPI_Instance n)
{
	SPIs[n]->MCR |= SPI_MCR_HALT_MASK;
}
void SPI_RunModule(SPI_Instance n)
{
	SPIs[n]->MCR &= ~SPI_MCR_HALT_MASK;
}
void SPI_ClearTxFIFO(SPI_Instance n)
{
	SPIs[n]->MCR |= SPI_MCR_CLR_TXF_MASK;
}
void SPI_StartCountingTxFIFO(SPI_Instance n)
{
	SPIs[n]->MCR &= ~SPI_MCR_CLR_TXF_MASK;
}
void SPI_SetEOQ(SPI_Instance n)
{
	SPIs[n]->MCR |= SPI_PUSHR_EOQ_MASK;
}
void SPI_ClearEOQ(SPI_Instance n)
{
	SPIs[n]->MCR &= ~SPI_PUSHR_EOQ_MASK;
}


uint32_t SPI_GetDataRegisterAddress(SPI_Instance n)
{
	return SPIs[n]->PUSHR;
}

bool SPI_SendByte( uint8_t byte)
{
	if(push(&transmitBuffer, &byte))
	{
		// Enable interrupts
		SPI_EnableTxFIFOFillInterruptRequests(SPI_0);
		return true;
	}
	else return false;
}



int SPI_SendFrame(uint8_t * data, uint8_t length, SPI_Callback callback)
{


	ASSERT(data!=NULL);
	ASSERT(length<spaceLeft(&transmitBuffer));

	int bytesSent = 0;
	for(int i=0; i< length; i++)
		if(push(&transmitBuffer, data+i)==false)
		{
			bytesSent = i;
			break;
		}

	if(bytesSent == 0) // If didn't break..
		bytesSent = length;

	// Store bytes left
	bytesLeft = bytesSent;

	transferCallback = callback;

	// Enable interrupts to start copying bytes from circular buffer to SPI module
	SPI_ClearTxFIFO(0);
	SPI_EnableTxFIFOFillInterruptRequests(SPI_0);
	return bytesSent;
}

bool SPI_ReceiveByte(uint8_t * byte)
{
	return pop(&recieveBuffer, byte);
}


void SPI0_IRQHandler(void)
{
	/// If EOQF bit is set, transmission has ended
	if((SPIs[0]->SR & SPI_SR_EOQF_MASK) == SPI_SR_EOQF_MASK)
	{SET_TEST_PIN;
		//SPIs[0]->MCR |= SPI_MCR_HALT_MASK;
		SPIs[0]->SR |= SPI_SR_EOQF_MASK; // Clear EOQF flag


		// Call user callback
		if(transferCallback!=NULL)
			transferCallback();
		CLEAR_TEST_PIN;

	}
	// If TFFF bit is set there is space in Tx FIFO
	if((SPIs[0]->SR & SPI_SR_TFFF_MASK) == SPI_SR_TFFF_MASK)
	{
		SPIs[0]->SR |= SPI_SR_TFFF_MASK;
		// If HALT bit is set, clear it to start transfer
		if((SPIs[0]->MCR & SPI_MCR_HALT_MASK) == SPI_MCR_HALT_MASK)
		{
			SPIs[0]->MCR &= ~SPI_MCR_HALT_MASK;
		}

		// Fill FIFO
		while(((SPIs[0]->SR&SPI_SR_TXCTR_MASK)>>SPI_SR_TXCTR_SHIFT) < 4 && numel(&transmitBuffer)>0 )
		{
			uint8_t byte;
			if(pop(&transmitBuffer, &byte))
			{
				// If its last byte, set end of queue bit
				if(isEmpty(&transmitBuffer))
				{
					SPIs[0]->PUSHR = SPI_PUSHR_CONT(0) |
								SPI_PUSHR_CTAS(0) |
								SPI_PUSHR_EOQ(1) |
								SPI_PUSHR_PCS(1) |
								SPI_PUSHR_TXDATA(byte);
					SPI_DisableTxFIFOFillRequests(SPI_0);
				}
				else
					SPIs[0]->PUSHR = SPI_PUSHR_CONT(1) |
								SPI_PUSHR_CTAS(0) |
								SPI_PUSHR_EOQ(0) |
								SPI_PUSHR_PCS(1) |
								SPI_PUSHR_TXDATA(byte);
			}
		}

	}

}

void receiveData(void)
{
	SPIs[0]->MCR |= SPI_MCR_HALT(1);
	SPIs[0]->MCR |= (SPI_MCR_DIS_TXF(config->disableTxFIFO) |			
			SPI_MCR_DIS_RXF(config->disableRxFIFO));
	SPIs[0]->SR |= (SPI_SR_TCF_MASK | SPI_SR_EOQF_MASK | SPI_SR_TFUF_MASK | SPI_SR_TFFF_MASK | SPI_SR_RFOF_MASK | SPI_SR_RFDF_MASK); //clear the status bits (write-1-to-clear)
 	SPIs[0]->TCR |= SPI_TCR_SPI_TCNT_MASK;
	SPIs[0]->MCR &=  ~SPI_MCR_HALT_MASK;
	SPIs[0]->PUSHR = (SPI_PUSHR_CONT_MASK |  SPI_PUSHR_PCS_MASK | 0x9F); //command byte

  	while (!(SPIs[0]->SR & SPI_SR_RFDF_MASK)  );

	SPI0_POPR; //dummy read

	SPIs[0]->SR = SPI_SR_RFDF_MASK;   // clear the reception flag (not self-clearing)

	SPIs[0]->PUSHR = (SPI_PUSHR_CONT_MASK | SPI_PUSHR_PCS0_ON | 0xFF); //dummy byte to read

	while (!(SPIs[0]->SR & SPI_SR_RFDF_MASK)  );

	receiveBuffer[0] = SPI0_POPR; //read

	SPIs[0]->SR = SPI_SR_RFDF_MASK;   // clear the reception flag (not self-clearing)

	SPIs[0]->PUSHR = (SPI_PUSHR_CONT_MASK |  SPI_PUSHR_PCS0_ON | 0xFF); //dummy byte to read

	while (!(SPIs[0]->SR & SPI_SR_RFDF_MASK)  );

	receiveBuffer[1] = SPI0_POPR; //read

	SPIs[0]->SR = SPI_SR_RFDF_MASK;   // clear the reception flag (not self-clearing)

	SPIs[0]->PUSHR = (SPI_PUSHR_EOQ_MASK | SPI_PUSHR_PCS0_ON | 0xFF); //send last dummy byte to read

	while (!(SPIs[0]->SR & SPI_SR_RFDF_MASK)  );

	receiveBuffer[2] = SPI0_POPR; //read

	SPIs[0]->SR = SPI_SR_RFDF_MASK;   // clear the reception flag (not self-clearing)
}


/*!
 * brief DSPI master transfer data using interrupts.
 *
 * This function transfers data using interrupts. This is a non-blocking function, which returns right away. When all
 * data is transferred, the callback function is called.

 * param base DSPI peripheral base address.
 * param handle Pointer to the dspi_master_handle_t structure which stores the transfer state.
 * param transfer Pointer to the dspi_transfer_t structure.
 * return status of status_t.
 */
    
int8_t DSPI_MasterTransferNonBlocking(SPI_Type *base, SPI_MasterHandle_t *handle, SPI_Transfer_t *transfer)
{
    /* If the transfer count is zero, then return immediately.*/
    if (transfer->dataSize == 0U || handle==NULL || transfer==NULL)
    {
        return -1;
    }

    /* Check that we're not busy.*/
    if (handle->state == (uint8_t)kDSPI_Busy)
    {
        return 0;
    }

    handle->state = (uint8_t)kDSPI_Busy;

    /* Disable the NVIC for DSPI peripheral. */
    (void)DisableIRQ(s_dspiIRQ[DSPI_GetInstance(base)]); //ESTO CREO QUE NO HACE FALTA, LO HARIA EN EL INIT Y FUE, QUE ESTE ACTIVO TODO EL TIEMPO

    DSPI_MasterTransferPrepare(base, handle, transfer); //ESTA ES UNA CAGADA, SOLAMENTE PREPARA TODO PARA ARRANCAR UNA TRANSMIION NUEVA, CREO QUE SE PUEDE REDUCIR BASTANTE SI USAMOS BUFFERS PROPIOS INTERNOS

    /* RX FIFO Drain request: RFDF_RE to enable RFDF interrupt
     * Since SPI is a synchronous interface, we only need to enable the RX interrupt.
     * The IRQ handler will get the status of RX and TX interrupt flags.
     */
    s_dspiMasterIsr = DSPI_MasterTransferHandleIRQ; //!ESTA ES MAS COMPLICADA, MANEJA TODO -> GANAS DE LLORAR INTENSIFIES

    DSPI_EnableInterrupts(base, (uint32_t)kDSPI_RxFifoDrainRequestInterruptEnable); //!ESTA QUEDA, ES PARA ACTIVAR QUE LAS INTERRUPCIONES DE LAS FIFO TE HAGAN SALTAR EL NVIC
    DSPI_StartTransfer(base); //ESTE ES SOLO CAMBIAR EL FLAG DE HALT

    /* Fill up the Tx FIFO to trigger the transfer. */
    DSPI_MasterTransferFillUpTxFifo(base, handle);  //ESTA NO SE QUE TAN NECESARIA ES

    /* Enable the NVIC for DSPI peripheral. */
    (void)EnableIRQ(s_dspiIRQ[DSPI_GetInstance(base)]); // ESTA CREO QUE SI LO HACEMOS EN EL INIT VUELA.

    return 1;
}

