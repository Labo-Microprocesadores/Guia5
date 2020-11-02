/***************************************************************************//**
  @file     timer.c
  @brief    Timer functions
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "timer.h"
#include "SysTick.h"

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
static int getArrayEffectiveLength (TimerElement timerElements [] );
static void Timer_PISR(void);

/*******************************************************************************
 * PRIVATE VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*A TimerElement's array to store the callbacks, their period, and other variables needed.*/
static TimerElement timerElements[INITIAL_TIMER_ELEMENTS_ARRAY_LENGTH];
/*A counter that avoid the repetition of the IDs returned by Timer_AddCallback*/
static int idCounter;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
bool Timer_Init (void)
{
	SysTick_Init();	//Initialization of Systick's driver.
	SysTick_AddCallback(&Timer_PISR, TIMER_ISR_PERIOD);	//Requests SysTick to periodically call the Timer's ISR.
	idCounter = 1;
	return true;
}

int Timer_AddCallback(void (*newCallback)(void), int period, bool callOnce)
{
	int quotient = (int) (period / TIMER_ISR_PERIOD);	//Calculates how many TIMER_ISR_PERIOD are equivalent to the callback period.

	if (quotient <= 0)
		return TimerPeriodError;	//period must be greater than TIMER_ISR_PERIOD.

	TimerElement newTimerElement = {idCounter,newCallback, quotient, 0, false, callOnce};	//Creates the new element
	timerElements[getArrayEffectiveLength(timerElements)] = newTimerElement;	//Stores the new element
	return idCounter++;
}

TimerError Timer_Delete(int timerID)
{
	bool idFound = false;	//Flag
	int i = 0;				//Index
	int arrayEffectiveLength = getArrayEffectiveLength(timerElements);	//Amount of not NULL elements in timerElements.
	/*Searches for the id in the array*/
	while((idFound == false) && (i < arrayEffectiveLength))
	{
		if(timerElements[i].callbackID == timerID)
		{
			idFound = true;//Flag ON
			/*Shifts the next elements one position (overwriting the element to be deleted)*/
			for(int j=i; j<((getArrayEffectiveLength(timerElements))-1); j++)
			{
				timerElements[j].callbackID = timerElements[j+1].callbackID;
				timerElements[j].callback = timerElements[j+1].callback;
				timerElements[j].counter = timerElements[j+1].counter;
				timerElements[j].paused = timerElements[j+1].paused;
				timerElements[j].counterLimit = timerElements[j+1].counterLimit;

			}

			/*Re-establishes the previous last position of the array to default values.*/
			timerElements[arrayEffectiveLength-1].callback = NULL;
			timerElements[arrayEffectiveLength-1].counter = 0;
			timerElements[arrayEffectiveLength-1].paused = false;
			timerElements[arrayEffectiveLength-1].callbackID = 0;
			timerElements[arrayEffectiveLength-1].counterLimit = 0;
		}
		i++;
	}
	if(idFound == false)
		return TimerNoIdFound;

	return TimerNoError;
}

TimerError Timer_Pause(int timerID)
{
	bool idFound = false;	//Flag
	int i = 0;				//Index

	/*Searches for the id in the array*/
	while((idFound == false) && (i < getArrayEffectiveLength(timerElements)))
	{
		if(timerElements[i].callbackID == timerID)
		{
			idFound = true;					//ID found
			timerElements[i].paused = true;	//Pauses the calling of the callback.
		}
		i++;
	};
	if(idFound == false)
		return TimerNoIdFound;

	return TimerNoError;

}


TimerError Timer_Resume(int timerID)
{
	bool idFound = false;	//Flag
	int i = 0;				//Index

	/*Searches for the id in the array*/
	while((idFound == false) && (i < getArrayEffectiveLength(timerElements)))
	{
		if(timerElements[i].callbackID == timerID)
		{
			idFound = true;			//ID found
			timerElements[i].paused = false; //Resumes the calling of the callback.
		}
		i++;
	} ;
	if(idFound == false)
		return TimerNoIdFound;

	return TimerNoError;
}

TimerError Timer_Reset(int timerID)
{
	bool idFound = false;	//Flag
	int i = 0;				//Index

	/*Searches for the id in the array*/
	while((idFound == false) && (i < getArrayEffectiveLength(timerElements)))
	{
		if(timerElements[i].callbackID == timerID)
		{
			idFound = true;			//ID found
			timerElements[i].counter = 0; //Resets the calling of the callback.
		}
		i++;
	} ;
	if(idFound == false)
		return TimerNoIdFound;

	return TimerNoError;
}

TimerError Timer_ChangePeriod(int timerID, int newPeriod)
{
	bool idFound = false;	//Flag
	int i = 0;				//Index

	/*Searches for the id in the array*/
	while((idFound == false) && (i < getArrayEffectiveLength(timerElements)))
	{
		if(timerElements[i].callbackID == timerID)
		{
			idFound = true;
			int quotient = (int) (newPeriod / TIMER_ISR_PERIOD);
			if(quotient<=0)
				return TimerPeriodError;	//newPeriod must be greater than SYSTICK_ISR_PERIOD

			timerElements[i].counterLimit = quotient;	//New counter limit.
			timerElements[i].counter = 0;				//Restarts counter.

		}
		i++;
	};
	if(idFound == false)
		return TimerNoIdFound;

	return TimerNoError;
}

float Timer_GetCallbackProgress(int timerID)
{
	bool idFound = false;	//Flag
	int i = 0;				//Index
	float progressFraction = -1.0;	//Error by default

	/*Searches for the id in the array*/
	while((idFound == false) && (i < getArrayEffectiveLength(timerElements)))
	{
		if(timerElements[i].callbackID == timerID)
		{
			idFound = true;					//ID found
			if (timerElements[i].counter == 0)
				progressFraction = 0;	//If the count of the callback hasn't started.
			else
				progressFraction =  (float)(timerElements[i].counter-1)/timerElements[i].counterLimit;	//timerElements[i].counter-1 because the counter counts the times that it enters the pISR. Therefore, the -1 converts the count to time intervals.
		}
		i++;
	};

	return progressFraction;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static int getArrayEffectiveLength (TimerElement timerElements[] )
{
	int i = 0;	//Index
	bool foundLast = false;	//Flag
	while (foundLast == false && i < INITIAL_TIMER_ELEMENTS_ARRAY_LENGTH)
	{

		if (timerElements[i].callback == NULL)	//Current element is null => The previous element was the last one.
			foundLast = true;
		else
			i++;
	}
	return i;
}


static void Timer_PISR(void)
{

	for(int i=0; i<(getArrayEffectiveLength(timerElements)); i++)	//Iterates through all the elements.
	{
		if (!timerElements[i].paused)
		{
			if(timerElements[i].counter == timerElements[i].counterLimit)	//If the counter reaches the counterLimit the element's callback must be called.
			{
				(*timerElements[i].callback)();	//Callback's calling.
				timerElements[i].counter = 0;	//Counter re-establishment.
				if (timerElements[i].callOnce)
					Timer_Delete(timerElements[i].callbackID);
			}
			timerElements[i].counter ++;
		}

	}
}
