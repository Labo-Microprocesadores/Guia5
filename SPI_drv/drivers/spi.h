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
enum SPIClockPolarity_t {ActiveHigh, ActiveLow};
enum SPIClockPhase_t {FirstEdge, SecondEdge};
enum SPIShiftDirection_t {MSBFirst, LSBFirst};
enum SPITxFIFOWatermark_t {TxFIFO0, TxFIFO1, TxFIFO2, TxFIFO3, TxFIFO4, TxFIFO5, TxFIFO6, TxFIFO7}; //Number of watermarked items.
enum SPIRxFIFOWatermark_t {RxFIFO1, RxFIFO2, RxFIFO3, RxFIFO4, RxFIFO5, RxFIFO6, RxFIFO7, RxFIFO8}; //Number of watermarked items.
enum SPIDataWidth_t {Data4Bits, Data5Bits, Data6Bits, Data7Bits, Data8Bits, Data9Bits, Data10Bits, Data11Bits, Data12Bits, Data13Bits, Data14Bits, Data15Bits, Data16Bits};
enum SPISlaveSelect_t {SlaveSelect0, SlaveSelect1, SlaveSelect2, SlaveSelect3};
enum SPISlavePolarity_t{};

/*Master Handle Config*/
 typedef struct
 {
    uint8_t *volatile txData;
    uint8_t *volatile rxData;
    volatile size_t 	txRemainingDataBytes;
    volatile size_t 	rxRemainingDataBytes;
    volatile size_t 	receiveDataRemainingBytesCount;
    size_t 	totalDataBytes;
    volatile uint32_t 	internalState;
    SPIMasterCallback_t 	masterCallback;
    void * 	callbackParam;
    uint8_t 	dataWidth; //1-16
    uint8_t 	slaveSelectNum; //0-3
    uint32_t 	configFlags; //OtherOptions
    SPITxFIFOWatermark_t txFIFOWatermark;
    SPIRxFIFOWatermark_t rxFIFOWatermark;
 }SPIMasterHandle_t;

/*Master Config*/
typedef struct
{
	bool enableMaster, enableLoopback;
	SPIClockPolarity_t polarity;
	SPIClockPhase_t phase;
	SPIShiftDirection_t direction;
  SPIDataWidth_t 	dataWidth;
  SPISlaveSelect_t slaveSelectNum;
  SPISlavePolarity_t 	sselPol;
  SPITxFIFOWatermark_t txFIFOWatermark;
  SPIRxFIFOWatermark_t rxFIFOWatermark;
  SPIDelayConfig_t delayConfig;
	uint32_t baudRateBps;
}SPIMasterConfig_t;


/*Slave Config*/
typedef struct
{
	bool enableSlave;
	SPIClockPolarity_t polarity;
	SPIClockPhase_t phase;
	SPIShiftDirection_t direction;
  SPIDataWidth_t 	dataWidth;
  SPISlavePolarity_t 	sselPol;
  SPITxFIFOWatermark_t txFIFOWatermark;
  SPIRxFIFOWatermark_t rxFIFOWatermark;
}SPISlaveConfig_t;


/*Delay config*/
typedef struct 
{
  uint8_t preDelay;
  uint8_t postDelay;
  uint8_t frameDelay;
  uint8_t transferDelay;
}SPIDelayConfig_t;

/*Transfer*/
typedef struct
{
  uint8_t * 	txData; //send buffer
  uint8_t * 	rxData; //receive buffer
  uint32_t 	configFlags; //Other options
  size_t 	dataSize; //bytes
}SPITransfer_t;

/*Half Duplex Transfer*/
typedef struct
{
  uint8_t * 	txData; //send buffer
  uint8_t * 	rxData; //receive buffer
  uint32_t 	configFlags; //Other options
  size_t 	txDataSize; //bytes
  size_t 	rxDataSize; //bytes
  bool isPinAssertBetweenTransmitAndReceive, doesTransmitFirst;
}SPIHalfDuplexTransfer_t;


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