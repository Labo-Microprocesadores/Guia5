/***************************************************************************//**
  @file     Led.h
  @brief    Led Header
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/

#ifndef LED_H_
#define LED_H_
/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "./board.h"
#include "./gpio.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define LED_ISR_PERIOD	100	//100ms
#define DEFAULT_LED_STATE	OFF
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum LedBlinkSpeed {NO_SPEED = 0, SLOW = 16, NORMAL = 6, FAST = 2} LedBlinkSpeed;
typedef enum LedState {ON = LOW, OFF = HIGH} LedState;
typedef enum LedID {LED_RED = PIN_LED_RED , LED_BLUE = PIN_LED_BLUE, LED_GREEN = PIN_LED_GREEN} LedID;
typedef enum LedError {LedNoError = 0, LedPeriodError = -1, LedNoIdFound = -2, LedOtherErrors = -3} LedError;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/**
 * @brief Initialization of the Led Driver.
 * @return true if no error occurred.
 */
bool Led_Init (void);

/**
 * @brief Turns a LED on.
 * @param ledID. The ID corresponding to the LED.
 */
void Led_On(LedID ledID);

/**
 * @brief Turns a LED off.
 * @param ledID. The ID corresponding to the LED.
 */
void Led_Off(LedID ledID);

/**
 * @brief Toggles a LED.
 * @param ledID. The ID corresponding to the LED.
 */
void Led_Toggle(LedID ledID);

/**
 * @brief 	Starts a "On For Defined Time" process.
 * 			A "On For Defined Time" process consists of a LED being ON during a defined time. After this time has elapsed, the LED is turned off.
 * @param ledID. The ID corresponding to the LED.
 * @param onTime. The time in ms. during which the LED must be ON in ms.
 * 					Example: onTime = 3000 is equivalent to 3000ms (3s)
 * WARNING If the quotient between onTime and LED_ISR_PERIOD is not an integer, it will be truncated.
 * @return A LedError indicating whether an error occurred (and its type) or not.
 */
LedError Led_OnForDefinedTime(LedID ledID, int onTime);

/**
 * @brief 	Starts a "Blink" process with custom properties.
 * 			A "Blink" process consists of a LED being turned on and off repeatedly. In other words, multiple "On For Defined Time" processes taking place repeatedly.
 * 			A "On For Defined Time" process consists of a LED being ON during a defined time. After this time has elapsed, the LED is turned off.
 * 			This function is "custom" because it gives the chance of determining the properties of the process described.
 * @param ledID. The ID corresponding to the LED.
 * @param blinkTimes. How many times the LED must be turned on and off.
 * @param blinkPeriod. The duration of an ON-OFF process during the blink in ms.
 * 							Example: blinkPeriod = 3000 is equivalent to 3000ms (3s)
 * @param onTime. The time in ms. during which the LED must be ON in an ON-OFF process.
 * @see Led_OnForDefinedTime.
 * WARNING If the quotient between blinkPeriod and LED_ISR_PERIOD is not an integer, it will be truncated.
 * @return A LedError indicating whether an error occurred (and its type) or not.
 */
LedError Led_CustomBlink(LedID ledID, int blinkTimes, int blinkPeriod, int onTime);

/**
 * @brief 	Starts a "Repetition Blink" process with custom properties.
 * 			A "Repetition Blink" process consists of multiple "Blink" processes taking place repeatedly.
 * 			A "Blink" process consists of a LED being turned on and off repeatedly. In other words, multiple "On For Defined Time" processes taking place repeatedly.
 * 			A "On For Defined Time" process consists of a LED being ON during a defined time. After this time has elapsed, the LED is turned off.
 * 			This function is "custom" because it gives the chance of determining the properties of the process described.
 * @param ledID. The ID corresponding to the LED.
 * @param repetitionTimes. How many times a "Blink" process must occur.
 * @param repetitionPeriod. The duration of an "Repetition Blink" in ms.
 * 							Example: repetitionPeriod = 3000 is equivalent to 3000ms (3s)
 * @param blinkTimesEachRepetition. How many times the LED must be turned on and off in each "Blink" process.
 * @param blinkPeriodEachRepetition. The duration of an ON-OFF process during each "Blink" process in ms.
 * @param onTimeEachBlink. The time in ms. during which the LED must be ON in each ON-OFF process in each "Blink" process.
 * @see Led_CustomBlink.
 * WARNING If the quotient between repetitionPeriod and LED_ISR_PERIOD is not an integer, it will be truncated.
 * @return A LedError indicating whether an error occurred (and its type) or not.
 */
LedError Led_CustomRepetitionBlink(LedID ledID, int repetitionTimes ,int repetitionPeriod, int blinkTimesEachRepetition, int blinkPeriodEachRepetition, int onTimeEachBlink);

/**
 * @brief 	Starts a "Infinite Blink" process.
 * 			A "Infinite Blink" process consists of "toggles" taking place repeatedly and infinitely or until the process is cancelled.
 * 			In this process, the ON and OFF time of each blink are the same.
 * @param ledID. The ID corresponding to the LED.
 * @param speed. The speed of the blink.
 * @return A LedError indicating whether an error occurred (and its type) or not.
 */
LedError Led_InfiniteBlink(LedID ledID, LedBlinkSpeed speed);

/**
 * @brief 	Stops a "Infinite Blink" process.
 * @param ledID. The ID corresponding to the LED.
 * @return A LedError indicating whether an error occurred (and its type) or not.
 */
LedError Led_StopInfiniteBlink(LedID ledId);

/**
 * @brief Stops all the processes ("On For Defined Time", "Blink", "Repetition Blink") and turns the LED to its default state.
 * @param ledID. The ID corresponding to the LED.
 * @return A LedError indicating whether an error occurred (and its type) or not.
 */
LedError Led_StopAllProcesses(LedID ledId);

/**
 * @brief Stops all the processes ("On For Defined Time", "Blink", "Repetition Blink") from all LEDS and turns them to their default state.
 */
void Led_StopAllProcessedFromAllLeds();
#endif /* LED_H_ */
