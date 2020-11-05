/***************************************************************************//**
  @file     spi.h
  @brief    SPI Driver for K64F.
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/


#ifndef _SPI_H_
#define _SPI_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SPI_CANT_IDS 3 // 0-2

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
 typedef struct spi_config_t
 {
     /* data */
 }spi_config_t;



typedef struct
{

	bool enableMaster, enableLoopback;
	SPI_CTAR CTARUsed;
	SPI_PCSignal PCSSignalSelect;
	SPI_BitsPerFrame bitsPerFrame;
	SPI_CLKDelayScaler clockDelayScaler, delayAfterTransfer, chipSelectToClkDelay;
	SPI_DelayAfterTransferPreScale delayAfterTransferPreScale;
	SPI_ChipSelectActiveState chipSelectActiveState;
	SPIClockPolarity_t polarity;
	SPIClockPhase_t phase;
	SPIShiftDirection_t direction;
  SPIDataWidth_t 	dataWidth;
	SPI_OutputMode outputMode;
	SPI_PinMode pinMode;
	uint32_t baudRateBps;
}SPIMasterConfig_t;


typedef struct 
{
  /* data */
}SPIClockPolarity_t;

typedef struct 
{
  /* data */
}SPIClockPhase_t;

typedef struct 
{
  /* data */
}SPIShiftDirection_t;

typedef struct 
{
  /* data */
}SPIDataWidth_t;




enum deviceRole{MASTER,SLAVE};
enum{SPI0,SPI1,SPI2};
enum{DISABLE,ENABLE};
enum{PULL_DOWN,PULL_UP,DISABLE_PULL};
enum{SPI_CONFIG,RESERVED_0,RESERVED_1,RESERVED_2};
enum{GPIO_ALTERNATIVE=1,SPI_ALTERNATIVE};
enum{PIN_PCS0,PIN_SCK_MASTER,PIN_MOSI_MASTER,PIN_MISO_MASTER};

 

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void SPI_init(uint8_t id, spi_config_t config, enum deviceRole role);

void SPI_send_msg(uint8_t id, const char* msg, uint8_t cant);

bool SPI_transfer_complete(uint8_t id);

bool SPI_is_msg(uint8_t id);

void SPI_read_msg(uint8_t id, const char* msg, uint8_t cant);

#endif //_SPI_H_