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
enum SPI_ClockPolarity_t {ActiveHigh, ActiveLow};
enum SPI_ClockPhase_t {FirstEdge, SecondEdge};
enum SPI_ShiftDirection_t {MSBFirst, LSBFirst};
enum SPI_TxFIFOWatermark_t {TxFIFO0, TxFIFO1, TxFIFO2, TxFIFO3, TxFIFO4, TxFIFO5, TxFIFO6, TxFIFO7}; //Number of watermarked items.
enum SPI_RxFIFOWatermark_t {RxFIFO1, RxFIFO2, RxFIFO3, RxFIFO4, RxFIFO5, RxFIFO6, RxFIFO7, RxFIFO8}; //Number of watermarked items.
enum SPI_DataWidth_t {Data4Bits, Data5Bits, Data6Bits, Data7Bits, Data8Bits, Data9Bits, Data10Bits, Data11Bits, Data12Bits, Data13Bits, Data14Bits, Data15Bits, Data16Bits};
enum SPI_SlaveSelect_t {SlaveSelect0, SlaveSelect1, SlaveSelect2, SlaveSelect3};
enum SPI_SlavePolarity_t{};
enum{SPI0,SPI1,SPI2} SPI_Type;
enum SPI_CTARSelection_t {CTAR0, CTAR1, CTAR2, CTAR3, CTAR4, CTAR5, CTAR6, CTAR7};
enum SPI_DelayType_t {PCtoSCK, LastSCKtoPC, BetweenTransfers};

/*Master Handle*/
 typedef struct
 {
    uint8_t *volatile txData;
    uint8_t *volatile rxData;
    volatile size_t 	txRemainingDataBytes;
    volatile size_t 	rxRemainingDataBytes;
    volatile size_t 	receiveDataRemainingBytesCount;
    size_t 	totalDataBytes;
    volatile uint32_t 	internalState;
    SPI_MasterCallback_t callback;
    void * 	callbackParam;
    uint8_t 	dataWidth; //1-16
    uint8_t 	slaveSelectNum; //0-3
    uint32_t 	configFlags; //OtherOptions
    SPI_TxFIFOWatermark_t txFIFOWatermark;
    SPI_RxFIFOWatermark_t rxFIFOWatermark;
 }SPI_MasterHandle_t;

 /*Slave Handle*/
typedef SPI_MasterHandle_t SPI_SlaveHandle_t;

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
}SPI_MasterConfig_t;


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
}SPI_SlaveConfig_t;


/*Delay config*/
typedef struct 
{
  uint8_t preDelay;
  uint8_t postDelay;
  uint8_t frameDelay;
  uint8_t transferDelay;
}SPI_DelayConfig_t;

/*Transfer*/
typedef struct
{
  uint8_t * 	txData; //send buffer
  uint8_t * 	rxData; //receive buffer
  uint32_t 	configFlags; //Other options
  size_t 	dataSize; //bytes
}SPI_Transfer_t;

/*Half Duplex Transfer*/
typedef struct
{
  uint8_t * 	txData; //send buffer
  uint8_t * 	rxData; //receive buffer
  uint32_t 	configFlags; //Other options
  size_t 	txDataSize; //bytes
  size_t 	rxDataSize; //bytes
  bool isPinAssertBetweenTransmitAndReceive, doesTransmitFirst;
}SPI_HalfDuplexTransfer_t;


enum deviceRole{MASTER,SLAVE};
enum{DISABLE,ENABLE};
enum{PULL_DOWN,PULL_UP,DISABLE_PULL};
enum{SPI_CONFIG,RESERVED_0,RESERVED_1,RESERVED_2};
enum{GPIO_ALTERNATIVE=1,SPI_ALTERNATIVE};
enum{PIN_PCS0,PIN_SCK_MASTER,PIN_MOSI_MASTER,PIN_MISO_MASTER};

//TODO CHEQUEAR:
 typedef void(* SPI_MasterCallback_t )(SPI_Type *base, SPI_MasterHandle_t *handle, int32_t status, void *userData);
 typedef void(* SPI_MasterTransferCallback_t)(SPI_Type *base, SPI_MasterHandle_t *handle, uint32_t status, void *userData);
typedef void(* SPI_SlaveTransferCallback_t)(SPI_Type *base, SPI_SlaveHandle_t *handle, uint32_t status, void *userData)

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void SPI_MasterInit	(	SPI_Type * 	base, const SPI_MasterConfig_t * 	masterConfig, uint32_t 	srcClockHz );	
void SPI_MasterGetDefaultConfig	(	SPI_MasterConfig_t * 	masterConfig	);
void SPI_SlaveInit	(	SPI_Type * 	base, const SPI_SlaveConfig_t * slaveConfig );	
void SPI_SlaveGetDefaultConfig	(	SPI_SlaveConfig_t * 	slaveConfig	);
void SPI_Deinit	(	SPI_Type * 	base	);	
uint32_t SPI_MasterSetBaudRate	(	SPI_Type * 	base, SPI_CTARSelection_t 	whichCtar, uint32_t 	baudRate_Bps, uint32_t 	srcClock_Hz );	
void SPI_MasterSetDelayScaler	(	SPI_Type * 	base, SPI_CTARSelection_t 	whichCtar, uint32_t 	prescaler, uint32_t 	scaler, SPI_DelayType_t 	whichDelay );	
void SPI_SlaveWriteDataBlocking	(	SPI_Type * 	base, uint32_t 	data );
void SPI_MasterTransferCreateHandle	(	SPI_Type * 	base, SPI_MasterHandle_t * 	handle, SPI_MasterTransferCallback_t 	callback, void * 	userData );	
void SPI_SlaveTransferCreateHandle	(	SPI_Type * 	base, SPI_SlaveHandle_t * 	handle, SPI_SlaveTransferCallback_t 	callback, void * 	userData );	
uint32_t SPI_MasterTransferBlocking	(	SPI_Type * 	base, SPI_Transfer_t * 	transfer );	
uint32_t SPI_MasterTransferNonBlocking	(	SPI_Type * 	base, SPI_MasterHandle_t * 	handle,SPI_Transfer_t * 	transfer );	
uint32_t DSPI_MasterTransferGetCount	(	SPI_Type * 	base, SPI_MasterHandle_t * 	handle, size_t * 	count );	
void SPI_MasterTransferAbort	(	SPI_Type * 	base, SPI_MasterHandle_t * 	handle )	;
void SPI_MasterTransferHandleIRQ	(	SPI_Type * 	base, SPI_MasterHandle_t * 	handle );	
uint32_t SPI_SlaveTransferNonBlocking	(	SPI_Type * 	base, SPI_SlaveHandle_t * 	handle, SPI_Transfer_t * 	transfer );	
uint32_t DSPI_SlaveTransferGetCount	(	SPI_Type * 	base, SPI_SlaveHandle_t * 	handle, size_t * 	count );	
void DSPI_SlaveTransferAbort	(	SPI_Type * 	base, SPI_SlaveHandle_t * 	handle );	
void DSPI_SlaveTransferHandleIRQ	(	SPI_Type * 	base, SPI_SlaveHandle_t * 	handle );	
void DSPI_MasterWriteCommandDataBlocking	(	SPI_Type * 	base, uint32_t 	data );	


void SPI_init(uint8_t id, spi_config_t config, enum deviceRole role);

void SPI_send_msg(uint8_t id, const char* msg, uint8_t cant);

bool SPI_transfer_complete(uint8_t id);

bool SPI_is_msg(uint8_t id);

void SPI_read_msg(uint8_t id, const char* msg, uint8_t cant);

#endif //_SPI_H_