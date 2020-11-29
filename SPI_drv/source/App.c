/***************************************************************************/ /**
  @file     App.c
  @brief    Application functions
  @author   Grupo 2
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "board.h"
#include "Timer.h"
#include "spi.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
/*
 * typedef struct
{
    bool enableMaster, enableRxFIFOverflowOverwrite, disableTxFIFO, disableRxFIFO;
    SPI_CTAR_t CTARUsed;
    SPI_PCSignal_t PCSSignalSelect;
    SPI_BitsPerFrame_t bitsPerFrame;
    SPI_ClockConfig_t clockConfig;
    SPI_SlaveSelectPolarity_t chipSelectPolarity;
    SPI_BitOrder_t bitOrder;
    uint8_t delayAfterTransfer;
    uint8_t clockDelayScaler;
    uint32_t baudRate;
} SPI_MasterConfig_t;
 *
 * typedef struct
{
    SPI_ClockPolarity_t clockPolarity;
    SPI_ClockPhase_t clockPhase;
    SPI_ClockScaler_t clockScaler;
} SPI_ClockConfig_t;

 *  */


/*******************************************************************************
 *******************************************************************************
                        FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
const uint16_t m[] = {'a', 'b', 'c', 'd'};
/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{
	static SPI_MasterConfig_t config = {true, false, false, false, SPI_CTAR_0, SPI_PCS_0, SPI_eightBitsFrame,
			{SPI_CLOCK_POLARITY_ACTIVE_HIGH, SPI_CLOCK_PHASE_FIRST_EDGE, SPI_CLOCK_SCALER_32}, SPI_SS_POLARITY_ACTIVE_HIGH,
			SPI_BIT_ORDER_MSB_FIRST, 1000, 1000, 15000};
	SPI_MasterInit(SPI_0, &config);



}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
	static bool var = true;
	static uint8_t recive[100];
	static uint8_t send[] = {21, 40};
	if(var)
	{
		//bool ret =  SPI_SendMessage(SPI_0, SPI_PCS_0, m, 4, false);
		spi_transaction(send, sizeof(send), recive);
		var = false;
	}
}



/*******************************************************************************
 *******************************************************************************/
