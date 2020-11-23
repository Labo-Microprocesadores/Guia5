#ifndef SPI_H_
#define SPI_H_

#include "stdint.h"
#include "stdbool.h"

typedef enum
{
	SPI_CTAR_0,
	SPI_CTAR_1
} SPI_CTAR;


/*SPI interrupt source.*/
typedef enum _dspi_interrupt_enable
{
    kDSPI_TxCompleteInterruptEnable         = (int)SPI_RSER_TCF_RE_MASK, /*!< TCF  interrupt enable.*/
    kDSPI_EndOfQueueInterruptEnable         = SPI_RSER_EOQF_RE_MASK,     /*!< EOQF interrupt enable.*/
    kDSPI_TxFifoUnderflowInterruptEnable    = SPI_RSER_TFUF_RE_MASK,     /*!< TFUF interrupt enable.*/
    kDSPI_TxFifoFillRequestInterruptEnable  = SPI_RSER_TFFF_RE_MASK,     /*!< TFFF interrupt enable, DMA disable.*/
    kDSPI_RxFifoOverflowInterruptEnable     = SPI_RSER_RFOF_RE_MASK,     /*!< RFOF interrupt enable.*/
    kDSPI_RxFifoDrainRequestInterruptEnable = SPI_RSER_RFDF_RE_MASK,     /*!< RFDF interrupt enable, DMA disable.*/
    kDSPI_AllInterruptEnable = (int)(SPI_RSER_TCF_RE_MASK | SPI_RSER_EOQF_RE_MASK | SPI_RSER_TFUF_RE_MASK |
                                     SPI_RSER_TFFF_RE_MASK | SPI_RSER_RFOF_RE_MASK | SPI_RSER_RFDF_RE_MASK)
    /*!< All above interrupts enable.*/
};

typedef enum
{
	SPI_PCS_0,
	SPI_PCS_1,
	SPI_PCS_2,
	SPI_PCS_3,
	SPI_PCS_4,
	SPI_PCS_5
} SPI_PCSignal;

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
} SPI_BitsPerFrame;

typedef enum
{
	SPI_onePowerDelay,
	SPI_twoPowerDelay,
	SPI_threePowerDelay,
	SPI_fourPowerDelay,
	SPI_fivePowerDelay,
	SPI_sixPowerDelay,
	SPI_sevenPowerDelay,
	SPI_eightPowerDelay,
	SPI_ninePowerDelay,
	SPI_tenPowerDelay
} SPI_CLKDelayScaler;

typedef enum
{
	SPI_DelayAfterTransferPreScaleOne,
	SPI_DelayAfterTransferPreScaleThree,
	SPI_DelayAfterTransferPreScaleFive,
	SPI_DelayAfterTransferPreScaleSeven
} SPI_DelayAfterTransferPreScale;

typedef enum
{
	SPI_PCSActiveHigh,
	SPI_PCSActiveLow,
} SPI_ChipSelectActiveState;

typedef enum
{
	SPI_ClockActiveHigh,
	SPI_ClockActiveLow,
} SPI_ClockPolarity;

typedef enum
{
	SPI_ClockPhaseFirstEdge,
	SPI_ClockPhaseSecondEdge,
} SPI_ClockPhase;

typedef enum
{
	SPI_FirstMSB,
	SPI_FirstLSB,
} SPI_ShiftDirection;

typedef enum
{
	SPI_SlaveSelectAsGPIO,
	SPI_SlaveSelectFaultPin,
	SPI_SlaveSelectAutomaticOutput,
} SPI_OutputMode;

typedef enum
{
	SPI_PinModeNormal,
	SPI_PinModeInput,
	SPI_PinModeOutput,
} SPI_PinMode;

typedef enum
{
	SPI_0,
	SPI_1,
	SPI_2
} SPI_Instance;

typedef struct
{
	bool enableMaster, enableStopInWaitMode, enableRxFIFOverflowOverwrite, disableTxFIFO, disableRxFIFO, continuousSerialCLK, continuousChipSelect;
	SPI_CTAR CTARUsed;
	SPI_PCSignal PCSSignalSelect;
	SPI_BitsPerFrame bitsPerFrame;
	SPI_CLKDelayScaler clockDelayScaler, delayAfterTransfer, chipSelectToClkDelay;
	SPI_DelayAfterTransferPreScale delayAfterTransferPreScale;
	SPI_ChipSelectActiveState chipSelectActiveState;
	SPI_ClockPolarity polarity;
	SPI_ClockPhase phase;
	SPI_ShiftDirection direction;
	SPI_OutputMode outputMode;
	SPI_PinMode pinMode;
	uint32_t baudRate;
} SPI_MasterConfig;

/* SPI master transfer handle struct */
typedef struct
{
	uint32_t bitsPerFrame;
	volatile uint32_t command;	   //Data command
	volatile uint32_t lastCommand; //Last data command

	uint8_t fifoSize; //Fifo datasize

	volatile bool
		isPcsActiveAfterTransfer; //After last frame transfer
	volatile bool isThereExtraByte;

	uint8_t *volatile txData;
	uint8_t *volatile rxData;
	volatile size_t remainingSendByteCount;
	volatile size_t remainingReceiveByteCount;
	size_t totalByteCount;

	volatile uint8_t state; //Transfer state

	SPI_Callback callback; //onComplete
	void *userData;		   //callback param
} SPI_MasterHandle_t;

/*Transfer struct.*/
typedef struct
{
	uint8_t *txData;
	uint8_t *rxData;
	volatile size_t dataSize;

	uint32_t configFlags; //Transfer config flags.
} SPI_Transfer_t;

enum SPI_TransferState
{
	SPI_IDLE_STATE = 0x0U,
	SPI_BUSY_STATE,
	SPI_ERROR_STATE /*!< Transfer error. */
};

void SPI_MasterInit(SPI_Instance n, SPI_MasterConfig *config);

typedef void (*SPI_Callback)(void);

void SPI_MasterGetDefaultConfig(SPI_MasterConfig *config);

void SPI_EnableTxFIFOFillDMARequests(SPI_Instance n);

void SPI_EnableTxFIFOFillInterruptRequests(SPI_Instance n);

void SPI_DisableTxFIFOFillRequests(SPI_Instance n);

void SPI_EnableEOQInterruptRequests(SPI_Instance n);

void SPI_DisableEOQInterruptRequests(SPI_Instance n);

uint32_t SPI_GetDataRegisterAddress(SPI_Instance n);

bool SPI_SendByte(uint8_t byte);

int SPI_SendFrame(uint8_t *data, uint8_t length, SPI_Callback callback);

#endif /* SPI_H_ */