#ifndef SPI_H_
#define SPI_H_

#include "stdint.h"
#include "stdbool.h"

// Clock polarity
typedef enum
{
    SPI_CLOCK_POLARITY_LOW,
    SPI_CLOCK_POLARITY_HIGH
} SPI_ClockPolarity_t;

// Clock phase
typedef enum
{
    SPI_CLOCK_PHASE_FIRST_EDGE,
    SPI_CLOCK_PHASE_SECOND_EDGE
} SPI_ClockPhase_t;

typedef enum
{
    SPI_CLOCK_SCALER_2 = 2,
    SPI_CLOCK_SCALER_4 = 4,
    SPI_CLOCK_SCALER_6 = 6,
    SPI_CLOCK_SCALER_8 = 8,
    SPI_CLOCK_SCALER_16 = 16,
    SPI_CLOCK_SCALER_32 = 32,
    SPI_CLOCK_SCALER_64 = 64,
    SPI_CLOCK_SCALER_128 = 128,
    SPI_CLOCK_SCALER_256 = 256,
    SPI_CLOCK_SCALER_512 = 512,
    SPI_CLOCK_SCALER_1024 = 1024,
    SPI_CLOCK_SCALER_2048 = 2048,
    SPI_CLOCK_SCALER_4096 = 4096,
    SPI_CLOCK_SCALER_8192 = 8192,
    SPI_CLOCK_SCALER_16384 = 16384,
    SPI_CLOCK_SCALER_32768 = 32768
} SPI_ClockScaler_t;

typedef struct
{
    SPI_ClockPolarity_t clockPolarity;
    SPI_ClockPhase_t clockPhase;
    SPI_ClockScaler_t clockScaler;
} SPI_ClockConfig_t;

typedef enum
{
    SPI_BIT_ORDER_MSB_FIRST,
    SPI_BIT_ORDER_LSB_FIRST,
} SPI_BitOrder_t;

typedef enum
{
    SPI_SS_POLARITY_ACTIVE_HIGH,
    SPI_SS_POLARITY_ACTIVE_LOW,
} SPI_SlaveSelectPolarity_t;

typedef enum
{
    SPI_PCS_0,
    SPI_PCS_1,
    SPI_PCS_2,
    SPI_PCS_3,
    SPI_PCS_4,
    SPI_PCS_5
} SPI_PCSignal_t;

typedef enum
{
    SPI_0,
    SPI_1,
    SPI_2,
} SPI_Instance_t;

typedef enum
{
    SPI_fourBitsFrame = 3,
    SPI_fiveBitsFrame,
    SPI_sixBitsFrame,
    SPI_sevenBitsFrame,
    SPI_eightBitsFrame,
    SPI_nineBitsFrame,
    SPI_tenBitsFrame,
    SPI_elevenBitsFrame,
    SPI_twelveBitsFrame,
    SPI_thirteenBitsFrame,
    SPI_fourteenBitsFrame,
    SPI_fifteenBitsFrame,
    SPI_sixteenBitsFrame
} SPI_BitsPerFrame_t;

typedef enum
{
    SPI_CTAR_0,
    SPI_CTAR_1
} SPI_CTAR_t;

typedef enum
{
    SPI_IDLE_STATE = 0x0U,
    SPI_BUSY_STATE,
    SPI_ERROR_STATE /*!< Transfer error. */
} SPI_TransferState_t;

typedef struct
{
    bool enableMaster, enableRxFIFOverflowOverwrite, disableTxFIFO, disableRxFIFO;
    SPI_CTAR_t CTARUsed;
    SPI_PCSignal_t PCSSignalSelect;
    SPI_BitsPerFrame_t bitsPerFrame;
    SPI_ClockConfig_t clockConfig;
    SPI_BitOrder_t bitOrder;
    uint32_t baudRate;
} SPI_MasterConfig_t;

void SPI_MasterInit(SPI_Instance_t n, SPI_MasterConfig_t *config);

typedef void (*SPI_onTransferCompleteCallback)(void);

bool SPI_SendByte(uint8_t byte);

int SPI_SendFrame(uint8_t *data, uint8_t length, SPI_onTransferCompleteCallback callback);

#endif /* SPI_H_ */
