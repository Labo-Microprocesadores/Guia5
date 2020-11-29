/***************************************************************************//**
  @file     Button.c
  @brief    Button configurations
  @author   Grupo 2
 ******************************************************************************/
/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "button.h"
#include "SysTick.h"
#include "gpio.h"

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static Button_t buttons[BUTTON_NUM];
bool var = false;

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void systick_callback(void)
{

	int i;
	//for the buttons array
	for( i=0 ; i<BUTTON_NUM ; i++ )
	{
		bool pinState = !gpioRead(buttons[i].pin);
		//if the was pressed and the button is not been pressed now
		if( buttons[i].lastState && !pinState)
		{
			buttons[i].wasTap = (buttons[i].currentCount < buttons[i].lkpTime);
			buttons[i].wasReleased = true;
			buttons[i].wasPressed = false;
			buttons[i].currentCount = 0;
			buttons[i].lastState = false;
		}
		// if the button is been pressed now
		else if( pinState )
		{
			//if the button is a long key press button and the currentCount is equal to the long key press time.
			if( buttons[i].typefunction == LKP && (++buttons[i].currentCount) == buttons[i].lkpTime )
			{
				buttons[i].wasLkp = true;
			}
			//if the button is a TYPEMATIC and the currentCount is equal to TYPEMATIC time.
			else if( buttons[i].typefunction == TYPEMATIC && ++buttons[i].currentCount == buttons[i].typeTime)
			{
				 buttons[i].wasPressed= true;
				 buttons[i].currentCount = 0;
			}
			else if(!buttons[i].lastState )
			{

				buttons[i].wasReleased = false;
				buttons[i].wasPressed = true;
				buttons[i].lastState = true;
			}
		}
	}
}


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void buttonsInit(void)
{
	//add buttons to .h
	SysTick_AddCallback(&systick_callback, 50);
}


bool wasPressed(pin_t button)
{
	int count;
	for(count=0;count<BUTTON_NUM;count++)
	{
		if(buttons[count].pin == button )
		{
			bool aux =buttons[count].wasPressed;
			if(aux)
				buttons[count].wasPressed = false;
			return aux ;
		}
	}
	return false;
}

bool wasTap(pin_t button)
{
	int count;
	for(count=0;count<BUTTON_NUM;count++)
	{
		if(buttons[count].pin == button )
		{
			bool aux =buttons[count].wasTap;
			if(aux)
				buttons[count].wasTap = false;
			return aux ;
		}
	}
	return false;
}


bool wasReleased(pin_t button)
{
	if(var==false)
	{
		int count;
			for(count=0;count<BUTTON_NUM;count++)
			{
				if(buttons[count].pin == button )
				{
					bool aux =buttons[count].wasReleased;
					if(aux)
						buttons[count].wasReleased = false;
					return aux ;
				}
			}
	}
	else
	{
		var=false;
		return false;
	}
	return false;
}

bool wasLkp(pin_t button)
{
	int count;
	for(count=0;count<BUTTON_NUM;count++)
	{
		if(buttons[count].pin == button )
		{
			bool aux =buttons[count].wasLkp;
			if(aux){
				var = true;
				buttons[count].wasLkp = false;
			}
			return aux ;
		}
	}
	return false;
}


bool buttonConfiguration(pin_t button, int type, int time)
{
	int count;
	//I move through the button arrangement and look for the same pin to reconfigure
	for(count=0;count<BUTTON_NUM;count++)
	{
		if(buttons[count].pin == button)
		{
			buttons[count].typefunction=type;
			if(type == LKP)
				buttons[count].lkpTime = time;
			else
				buttons[count].typeTime = time;
			return true;
		}
	}
	//if the pin was not there use an empty space
	for(count=0;count<BUTTON_NUM;count++)
	{
		if(buttons[count].pin==0)
		{
			gpioMode(button,INPUT);
			buttons[count].pin=button;
			buttons[count].typefunction=type;
			if(type == LKP)
				buttons[count].lkpTime = time;
			else
				buttons[count].typeTime = time;
			return true;
		}
	}
	//if there is no empty space for the value
	return false;
}

