/***************************************************************************/ /**
  @file     SysTick.h
  @brief    SysTick Header
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define SYSTICK_ISR_PERIOD 100000L //1ms
#define INITIAL_SYSTICK_ELEMENTS_ARRAY_LENGTH 20
#define MS_TO_TICK_CONVERTION 100000 //1ms
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/* Each SysTickElement is used to store the callback which SysTick needs to call.
 * @variable callbackID. An unique ID of the element.
 * @variable callback. The function to be called.
 * @variable counterLimit.  The quotient between the callback period and the Systick's ISR period.
 * 							Indicates the amount of times the Systick's ISR must occur before calling the callback.
 * @variable counter. Indicates the amount of times the Systick's ISR occurred. It's reestablished when counterLimit is reached.
 * @variable paused. Indicates whether the calling of a callback is paused or not.
 */
typedef struct SysTickElement
{
	int callbackID;
	void (*callback)(void);
	int counterLimit;
	int counter;
	bool paused;
} SysTickElement;

typedef enum SystickError
{
	SystickNoError = 0,
	SystickPeriodError = -1,
	SystickNoIdFound = -2
} SystickError;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialization of the SysTick Driver.
 * @return true if no error occurred.
 */
bool SysTick_Init(void);

/**
 * @brief Adds a callback to be periodically called by SysTick.
 * @param newCallback The function to be called. Must receive and return void. Usually a PISR.
 * @param period The period (in ms.) with which the callback is called. Must be greater than SYSTICK_ISR_PERIOD (or equal).
 * 			Example: newPeriod = 2000 is equivalent to 2ms
 * @return 	An ID to represent the callback element if no error occurred.
 * 			Must use this ID in case the calling needs to be cancelled or the period needs to be changed.
 * 			WARNING: if the returning number is negative it indicates an error and must be interpreted as a SystickError element.
 * WARNING If the quotient between period and SYSTICK_ISR_PERIOD is not an integer, it will be truncated.
 */
int SysTick_AddCallback(void (*newCallback)(void), int period);

/**
 * @brief Cancels the calling of a callback by SysTick.
 * @param id The callback ID given by Systick_AddCallback.
 * @return A SystickError indicating whether an error occurred (and its type) or not.
 */
SystickError Systick_ClrCallback(int id);

/**
 * @brief Pauses the calling of a callback by SysTick.
 * @param id The callback ID given by Systick_AddCallback.
 * @return A SystickError indicating whether an error occurred (and its type) or not.
 */
SystickError Systick_PauseCallback(int id);

/**
 * @brief Resumes the calling of a callback by SysTick following a predefined period (defined in SysTick_AddCallback).
 * @param id The callback ID given by Systick_AddCallback.
 * @return A SystickError indicating whether an error occurred (and its type) or not.
 */
SystickError Systick_ResumeCallback(int id);

/**
 * @brief Changes the period of calling of a callback.
 * @param id The callback ID given by Systick_AddCallback.
 * @param newPeriod The new period (in ms.) with which the callback is called. Must be greater than SYSTICK_ISR_PERIOD (or equal).
 * 				Example: newPeriod = 2000 is equivalent to 2ms
 * @return A SystickError indicating whether an error occurred (and its type) or not.
 * WARNING If the quotient between newPeriod and SYSTICK_ISR_PERIOD is not an integer, it will be truncated.
 */
SystickError Systick_ChangeCallbackPeriod(int id, int newPeriod);

/*******************************************************************************
 ******************************************************************************/
#endif
