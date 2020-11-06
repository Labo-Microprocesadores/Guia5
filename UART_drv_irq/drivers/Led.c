/***************************************************************************/ /**
  @file     led.c
  @brief    Led functions
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "led.h"
#include "timer.h"

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/** Structure to store the variables needed to make the "On For Defined Time" process work.
 * @variable onTime. The time in ms. for the LED to be ON.
 * @variable onTimeCounterLimit. 	The quotient between the "on" time and the Led's ISR period.
* 									Indicates the amount of times the Led's ISR must occur before the LED turns off.
 * @variable onTimeCounter.   Indicates the amount of times the Led's ISR occurred. It's reestablished when onTimeCounterLimit is reached.
 * @see Led_OnForDefinedTime.
 */
typedef struct OnForDefinedTimeProperties
{
	unsigned int onTime;
	unsigned int onTimeCounterLimit;
	unsigned int onTimeCounter;
} OnForDefinedTimeProperties;

/** Structure to store the variables needed to make the "Blink" process work.
 * @variable blinkTimes. How many times the LED must turn on.
 * @variable blinkCounter. How many times the LED has turned on.
 * @variable blinkPeriod. The duration of an ON-OFF process during the blink.
 * @variable blinkTimeCounterLimit. The quotient between the blink period and the Led's ISR period.
 * 									Indicates the amount of times the Led's ISR must occur before a blink (ON-OFF process) finishes.
 * @variable blinkTimeCounter. Indicates the amount of times the Led's ISR occurred. It's reestablished when blinkTimeCounterLimit is reached.
 * @see Led_CustomBlink.
 */
typedef struct BlinkProperties
{
	int blinkTimes;
	int blinkCounter;
	int blinkPeriod;
	int blinkTimeCounterLimit;
	int blinkTimeCounter;
} BlinkProperties;

/** Structure to store the variables needed to make the "Repetition Blink" process work.
 * @variable repetitionTimes. How many times a blink process must occur.
 * @variable repetitionCounter. How many times  a blink process has occurred.
 * @variable repetitionPeriod. The duration of a repetition blink process. The sum of the blinking and the following OFF time.
 * @variable repetitionBlinkTimeCounterLimit. The quotient between the repetition blink period and the Led's ISR period.
 * 											Indicates the amount of times the Led's ISR must occur before a repetition blink (sum of ON-OFF processes + extra OFF time) finishes.
 * @variable repetitionBlinkTimeCounter. Indicates the amount of times the Led's ISR occurred. It's reestablished when repetitionBlinkTimeCounterLimit is reached.
 * @see Led_CustomRepetitionBlink.
 */
typedef struct RepetitionBlinkProperties
{
	int repetitionTimes;
	int repetitionCounter;
	int repetitionPeriod;
	int repetitionBlinkTimeCounterLimit;
	int repetitionBlinkTimeCounter;
} RepetitionBlinkProperties;

/** Structure to store the variables needed to make the "Infinite Blink" process work.
 * @variable blinkSpeed. he speed of the blink.
 * @variable togglePeriod. The duration of an ON or OFF process during the blink.
 * @variable toggleTimeCounterLimit. 	The quotient between the togglePeriod and the Led's ISR period.
 * 										Indicates the amount of times the Led's ISR must occur before toggle.
 * @variable toggleTimeCounter. Indicates the amount of times the Led's ISR occurred. It's reestablished when toggleTimeCounterLimit is reached.
 */
typedef struct InfiniteBlinkProperties
{
	LedBlinkSpeed blinkSpeed;
	int toggleTimeCounterLimit;
	int toggleTimeCounter;
} InfiniteBlinkProperties;

/** Structure to store the variables of a LED.
 * @variable ledID. An id indicating to which LED the structure corresponds.
 * @variable isOnForDefinedTime. Flag indicating whether a "On For Defined Time" process is taking place or not.
 * @variable isBlinking. Flag indicating whether a "Blink" process is taking place or not.
 * @variable isRepeatedlyBlinking. Flag indicating whether a "Repetition Blink" process is taking place or not.
 * @variable isInfinitelyBlinking. Flag indicating whether a "Infinite Blink" process is taking place or not.
 * @variable infiniteBlinkProperties.  Structure storing the variables needed to make the "Infinite Blink" process work.
 * @variable onForDefinedTimeProperties. Structure storing the variables needed to make the "On For Defined Time" process work.
 * @variable blinkProperties. Structure storing the variables needed to make the "Blink" process work.
 * @variable repetitionBlinkProperties. Structure storing the variables needed to make the "Repetition Blink" process work.
 * @see Led_OnForDefinedTime.
 * @see Led_CustomBlink.
 * @see Led_CustomRepetitionBlink.
 * @see Led_InfiniteBlink
 */
typedef struct LedElement
{
	LedID ledID;
	bool isOnForDefinedTime;
	bool isBlinking;
	bool isRepeatedlyBlinking;
	bool isInfinitelyBlinking;
	InfiniteBlinkProperties infiniteBlinkProperties;
	OnForDefinedTimeProperties onForDefinedTimeProperties;
	BlinkProperties blinkProperties;
	RepetitionBlinkProperties repetitionBlinkProperties;
} LedElement;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
/**
 * @brief 	Searches for a LedElement with a corresponding ID in the ledElements array and returns its index if found.
 * 			If the ID is not found, returns -1.
 * @param id The id of the LED to be found.
 * @return The index of the LedElement associated with the id in the ledElements array.
 */
static int findLedIndexByID(LedID id);

/**
 * @brief Manages the time and events of the processes taking place in each LED.
 */
static void Led_PISR(void);
/**
 * @brief Manages all the related with the On For Defined Time processes each cycle of the PISR.
 */
static void OnForDefinedTimeCalledFromPISR(LedElement *ledElement);
/**
 * @brief Manages all the related with the Blink processes each cycle of the PISR.
 */
static void BlinkCalledFromPISR(LedElement *ledElement);
/**
 * @brief Manages all the related with the Repetition Blink processes each cycle of the PISR.
 */
static void RepetitionBlinkCalledFromPISR(LedElement *ledElement);
/**
 * @brief Manages all the related with the Infinite Blink processes each cycle of the PISR.
 */
static void InfiniteBlinkCalledFromPISR(LedElement *ledElement);


/*******************************************************************************
* PRIVATE VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/** A LedElement's array to store the information and state of each led.*/
static LedElement ledElements[3];
/** Default structures for the LEDs to be created with.
 * These may change if a "On For Defined Time", "Blink", or "Repetition Blink" process is requested.
 */
static OnForDefinedTimeProperties defaultOnForDefinedTimeProperties = {0, 0, 0};
static BlinkProperties defaultBlinkProperties = {0, 0, 0, 0, 0};
static InfiniteBlinkProperties defaultInfiniteBlinkProperties = {NO_SPEED, 0, 0};
static RepetitionBlinkProperties defaultRepetitionBlinkProperties = {0, 0, 0, 0, 0};

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool Led_Init(void)
{
	/*LEDs are defined as OUTPUT*/
	gpioMode(LED_RED, OUTPUT);
	gpioMode(LED_GREEN, OUTPUT);
	gpioMode(LED_BLUE, OUTPUT);

	/*LEDs are turned to its default state (generally OFF).*/
	gpioWrite(LED_RED, DEFAULT_LED_STATE);
	gpioWrite(LED_GREEN, DEFAULT_LED_STATE);
	gpioWrite(LED_BLUE, DEFAULT_LED_STATE);

	/*Each led is created as a LedElement with the default structures.*/
	LedElement redLed = {LED_RED, false, false, false, false, defaultInfiniteBlinkProperties, defaultOnForDefinedTimeProperties, defaultBlinkProperties, defaultRepetitionBlinkProperties};
	LedElement greenLed = {LED_GREEN, false, false, false, false, defaultInfiniteBlinkProperties, defaultOnForDefinedTimeProperties, defaultBlinkProperties, defaultRepetitionBlinkProperties};
	LedElement blueLed = {LED_BLUE, false, false, false, false, defaultInfiniteBlinkProperties, defaultOnForDefinedTimeProperties, defaultBlinkProperties, defaultRepetitionBlinkProperties};

	/*The LedElements are added to the array of LEDs*/
	ledElements[0] = redLed;
	ledElements[1] = greenLed;
	ledElements[2] = blueLed;

	Timer_AddCallback(&Led_PISR, LED_ISR_PERIOD, false); //Requests Timer to periodically call the Led's ISR.
	return true;
}

void Led_On(LedID ledID)
{
	gpioWrite(ledID, ON);
}

void Led_Off(LedID ledID)
{
	gpioWrite(ledID, OFF);
}

void Led_Toggle(LedID ledID)
{
	gpioToggle(ledID);
}

LedError Led_OnForDefinedTime(LedID ledID, int onTime)
{
	int indexInArray = findLedIndexByID(ledID); //Finds the LED index in the ledElements array.
	if (indexInArray == -1)
		return LedNoIdFound;

	int quotient = (int)(onTime / LED_ISR_PERIOD); //Calculates how many LED_ISR_PERIOD are equivalent to the onTime period.

	if (quotient <= 0)
		return LedPeriodError; //period must be greater than LED_ISR_PERIOD.

	/*Defines a "On For Defined Time" process variables.*/
	ledElements[indexInArray].onForDefinedTimeProperties.onTime = onTime;
	ledElements[indexInArray].onForDefinedTimeProperties.onTimeCounterLimit = quotient; //Indicates the amount of times the Led's ISR must occur before the LED turns off.
	ledElements[indexInArray].onForDefinedTimeProperties.onTimeCounter = 0;				//Counts the amount of times the Led's ISR occurred.

	ledElements[indexInArray].isOnForDefinedTime = true; //Flag indicating the process is taking place.
	return LedNoError;
}

LedError Led_CustomBlink(LedID ledID, int blinkTimes, int blinkPeriod, int onTime)
{
	int indexInArray = findLedIndexByID(ledID); //Finds the LED index in the ledElements array.
	if (indexInArray == -1)
		return LedNoIdFound;

	if (onTime > blinkPeriod)
		return LedPeriodError;

	int quotient = (int)(blinkPeriod / LED_ISR_PERIOD); //Calculates how many LED_ISR_PERIOD are equivalent to a blinkPeriod.
	if (quotient <= 0)
		return LedPeriodError; //blinkPeriod must be greater than LED_ISR_PERIOD.

	/** Defines a "Blink" process variables.
	 * A "Blink" process consists of a LED being turned on and off repeatedly. In other words, multiple "On For Defined Time" processes taking place repeatedly.
	 * A "On For Defined Time" process consists of a LED being ON during a defined time. After this time has elapsed, the LED is turned off.
	 */
	ledElements[indexInArray].blinkProperties.blinkPeriod = blinkPeriod;
	ledElements[indexInArray].blinkProperties.blinkTimes = blinkTimes;
	ledElements[indexInArray].blinkProperties.blinkCounter = 0;
	ledElements[indexInArray].blinkProperties.blinkTimeCounterLimit = quotient;
	ledElements[indexInArray].blinkProperties.blinkTimeCounter = 0;
	/*"On For Defined Time" subprocesses properties*/
	ledElements[indexInArray].onForDefinedTimeProperties.onTime = onTime;

	ledElements[indexInArray].isBlinking = true; //Flag indicating the process is taking place.

	//Requests a "On For Defined Time" process (starts a "Blink" process).

	return LedNoError;
}

LedError Led_CustomRepetitionBlink(LedID ledID, int repetitionTimes, int repetitionPeriod, int blinkTimesEachRepetition, int blinkPeriodEachRepetition, int onTimeEachBlink)
{

	int indexInArray = findLedIndexByID(ledID); //Finds the LED index in the ledElements array.
	if (indexInArray == -1)
		return LedNoIdFound;

	if (blinkPeriodEachRepetition > repetitionPeriod || onTimeEachBlink > blinkPeriodEachRepetition)
		return LedPeriodError;

	int quotient = (int)(repetitionPeriod / LED_ISR_PERIOD); //Calculates how many LED_ISR_PERIOD are equivalent to a repetitionPeriod.
	if (quotient <= 0)
		return LedPeriodError; //repetitionPeriod must be greater than LED_ISR_PERIOD.

	/** Defines a "Repetition Blink" process variables.
	 * A "Repetition Blink" process consists of multiple "Blink" processes taking place repeatedly.
	 * A "Blink" process consists of a LED being turned on and off repeatedly. In other words, multiple "On For Defined Time" processes taking place repeatedly.
	 * A "On For Defined Time" process consists of a LED being ON during a defined time. After this time has elapsed, the LED is turned off.
	 */
	ledElements[indexInArray].repetitionBlinkProperties.repetitionTimes = repetitionTimes; //How many times a "Blink" process must occur.
	ledElements[indexInArray].repetitionBlinkProperties.repetitionCounter = 0;
	ledElements[indexInArray].repetitionBlinkProperties.repetitionPeriod = repetitionPeriod;
	ledElements[indexInArray].repetitionBlinkProperties.repetitionBlinkTimeCounterLimit = quotient;
	ledElements[indexInArray].repetitionBlinkProperties.repetitionBlinkTimeCounter = 0;

	/*"Blink" subprocesses properties*/
	ledElements[indexInArray].blinkProperties.blinkPeriod = blinkPeriodEachRepetition;
	ledElements[indexInArray].blinkProperties.blinkTimes = blinkTimesEachRepetition;
	ledElements[indexInArray].blinkProperties.blinkCounter = 0;

	/*"On For Defined Time" subprocesses properties*/
	ledElements[indexInArray].onForDefinedTimeProperties.onTime = onTimeEachBlink;

	ledElements[indexInArray].isRepeatedlyBlinking = true; //Flag indicating the process is taking place.
	return LedNoError;
}

LedError Led_InfiniteBlink(LedID ledID, LedBlinkSpeed speed)
{
	if (speed == NO_SPEED)
		return LedOtherErrors;
	int indexInArray = findLedIndexByID(ledID); //Finds the LED index in the ledElements array.
	if (indexInArray == -1)
		return LedNoIdFound;

	InfiniteBlinkProperties properties = {speed, speed, 0}; //LedBlinkSpeed is defined as an enum. Each element indicates the quotient between the toggle period and the LED_ISR_PERIOD.
	ledElements[indexInArray].infiniteBlinkProperties = properties;
	ledElements[indexInArray].isInfinitelyBlinking = true;

	return LedNoError;
}

LedError Led_StopInfiniteBlink(LedID ledId)
{
	int indexInArray = findLedIndexByID(ledId); //Finds the LED index in the ledElements array.
	if (indexInArray == -1)
		return LedNoIdFound;

	ledElements[indexInArray].isInfinitelyBlinking = false;
	ledElements[indexInArray].infiniteBlinkProperties = defaultInfiniteBlinkProperties;
	gpioWrite(ledId, DEFAULT_LED_STATE);
	return LedNoError;
}

LedError Led_StopAllProcesses(LedID ledId)
{
	int indexInArray = findLedIndexByID(ledId); //Finds the LED index in the ledElements array.
	if (indexInArray == -1)
		return LedNoIdFound;

	/*Sets processes' flags to false*/
	ledElements[indexInArray].isRepeatedlyBlinking = false;
	ledElements[indexInArray].isBlinking = false;
	ledElements[indexInArray].isOnForDefinedTime = false;
	ledElements[indexInArray].isInfinitelyBlinking = false;

	/*Resets the properties of the processes*/
	ledElements[indexInArray].infiniteBlinkProperties = defaultInfiniteBlinkProperties;
	ledElements[indexInArray].blinkProperties = defaultBlinkProperties;
	ledElements[indexInArray].repetitionBlinkProperties = defaultRepetitionBlinkProperties;
	ledElements[indexInArray].onForDefinedTimeProperties = defaultOnForDefinedTimeProperties;

	/*Turns the LED to its default state.*/
	gpioWrite(ledId, DEFAULT_LED_STATE);

	return LedNoError;
}

void Led_StopAllProcessedFromAllLeds()
{
	for (int i = 0; i < sizeof(ledElements) / sizeof(ledElements[0]); i++) //Iterates through each LED
	{
		Led_StopAllProcesses(ledElements[i].ledID);
	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static int findLedIndexByID(LedID id)
{
	for (int i = 0; i < sizeof(ledElements) / sizeof(ledElements[0]); i++) //Iterates through all the elements.
	{
		if (ledElements[i].ledID == id) //If the id is found, returns the index.
			return i;
	}
	return -1; //Not found
}

static void Led_PISR(void)
{
	for (int i = 0; i < sizeof(ledElements) / sizeof(ledElements[0]); i++) //Iterates through each LED
	{
		if (ledElements[i].isInfinitelyBlinking) //Checks if a "Infinite Blink" process is taking place.
		{
			InfiniteBlinkCalledFromPISR(&ledElements[i]);
		}

		if (ledElements[i].isRepeatedlyBlinking) //Checks if a "Repetition Blink" process is taking place.
		{
			RepetitionBlinkCalledFromPISR(&ledElements[i]);
		}

		if (ledElements[i].isBlinking) //Checks if a "Blink" process is taking place.
		{
			BlinkCalledFromPISR(&ledElements[i]);
		}
		if (ledElements[i].isOnForDefinedTime) //Checks if a "On For Defined Time" process is taking place.
		{
			OnForDefinedTimeCalledFromPISR(&ledElements[i]);
		}
	}
}

static void OnForDefinedTimeCalledFromPISR(LedElement *ledElement)
{
	if (ledElement->onForDefinedTimeProperties.onTimeCounter == 0) //Enters for the first time.
		Led_On(ledElement->ledID);								   //Turns the LED on (starts the process).
	/*Checks if the process has to be finished*/
	if (ledElement->onForDefinedTimeProperties.onTimeCounter == ledElement->onForDefinedTimeProperties.onTimeCounterLimit)
	{
		//Finishes the process and resets variables.
		ledElement->isOnForDefinedTime = false;
		ledElement->onForDefinedTimeProperties.onTimeCounter = 0;
		Led_Off(ledElement->ledID); //Turns the LED off (finishes the process).
	}
	ledElement->onForDefinedTimeProperties.onTimeCounter++;
}

static void BlinkCalledFromPISR(LedElement *ledElement)
{
	/*If it enters for the first time, the process is started by requesting a "Led On For Defined Time" subprocess*/
	if (ledElement->blinkProperties.blinkTimeCounter == 0)										//If a subprocess has to start.
		Led_OnForDefinedTime(ledElement->ledID, ledElement->onForDefinedTimeProperties.onTime); //New subprocess
	/*Checks if a "Led On For Defined Time" subprocess has finished*/
	else if (ledElement->blinkProperties.blinkTimeCounter == ledElement->blinkProperties.blinkTimeCounterLimit) //If a subprocess has to end.
	{
		//Finishes the "On For Defined Time" subprocess and resets variables.
		ledElement->blinkProperties.blinkTimeCounter = 0;
		/*Checks if the "Blink" process has completed*/
		if (++ledElement->blinkProperties.blinkCounter == ledElement->blinkProperties.blinkTimes) //If the process has to end.
			ledElement->isBlinking = false;														  //Finishes the process.
		else																					  //If the process hasn't completed, it requests another "Led On For Defined Time" subprocess
			Led_OnForDefinedTime(ledElement->ledID, ledElement->onForDefinedTimeProperties.onTime);
	}
	ledElement->blinkProperties.blinkTimeCounter++;
}

static void RepetitionBlinkCalledFromPISR(LedElement *ledElement)
{
	/*If it enters for the first time, the process is started by requesting a "Blink" subprocess*/
	if (ledElement->repetitionBlinkProperties.repetitionBlinkTimeCounter == 0)
		Led_CustomBlink(ledElement->ledID, ledElement->blinkProperties.blinkTimes, ledElement->blinkProperties.blinkPeriod, ledElement->onForDefinedTimeProperties.onTime);
	/*Checks if a "Blink" subprocess has finished*/
	else if (ledElement->repetitionBlinkProperties.repetitionBlinkTimeCounter == ledElement->repetitionBlinkProperties.repetitionBlinkTimeCounterLimit)
	{
		//Finishes the "Blink" subprocess and resets variables.
		ledElement->repetitionBlinkProperties.repetitionBlinkTimeCounter = 0;
		/*Checks if the "Repetition Blink" process has completed*/
		if (++ledElement->repetitionBlinkProperties.repetitionCounter == ledElement->repetitionBlinkProperties.repetitionTimes)
			ledElement->isRepeatedlyBlinking = false; //Finishes the process.
		else										  //If the process hasn't completed, it requests another "Blink" subprocess
			Led_CustomBlink(ledElement->ledID, ledElement->blinkProperties.blinkTimes, ledElement->blinkProperties.blinkPeriod, ledElement->onForDefinedTimeProperties.onTime);
	}
	ledElement->repetitionBlinkProperties.repetitionBlinkTimeCounter++;
}

static void InfiniteBlinkCalledFromPISR(LedElement *ledElement)
{
	if (ledElement->infiniteBlinkProperties.toggleTimeCounter == 0) //Enters for the first time.
		Led_Toggle(ledElement->ledID);								//Toggles the LED			//Starts the process.

	/*Checks if a toggle needs to be done.*/
	else if (ledElement->infiniteBlinkProperties.toggleTimeCounter == ledElement->infiniteBlinkProperties.toggleTimeCounterLimit) //Reaches the time limit.
	{
		ledElement->infiniteBlinkProperties.toggleTimeCounter = 0; //Restart
		Led_Toggle(ledElement->ledID);							   //Toggles the LED
	}
	ledElement->infiniteBlinkProperties.toggleTimeCounter++; //Next time count.
}
