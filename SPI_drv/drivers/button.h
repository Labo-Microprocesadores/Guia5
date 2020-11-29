/***************************************************************************//**
  @file     Button.h
  @brief    Button configurations
  @author   Grupo 2
 ******************************************************************************/

#ifndef BUTTON_H_
#define BUTTON_H_
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define BUTTON_NUM 1
#define TIME_BASE 3

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

	enum type{NORMAL_E,TYPEMATIC, LKP};

  /** Structure to store the variables needed to define a button object.
 * @variable pin number of the pin used for this button 
 * @variable enum with the working modes (NORMAL,TYPEMATIC, LKP)
 * @variable last state, variable that records the las state of the button 
 * @variable currentCount button counter
 * @variable lkpTime time to consider the tap as a long key press
 * @variable typeTime time if is not a long key press
 * @variable wasLkp variable that registers the tap on longkeypress mode
 * @variable wasPressed variable that registers the touch of the button
 * @variable was released variable that registers the release of the button
 */
typedef  struct
{
	pin_t pin;
	enum type typefunction;
	bool lastState;
	int currentCount;
	int lkpTime;
	int typeTime;
	bool wasTap;
	bool wasLkp;
	bool wasPressed;
	bool wasReleased;
}Button_t;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialization of the Button Driver.
 * @return true if no error occurred.
 */
void buttonsInit(void);

/**
 * @brief function that goes through all elements in the buttons array and when it finds the button that was passed as an argument
 * 		 return the value of wasPressed for the button and if it was pressed -> change the variable was pressed to false (restore the value)
 * @param button, button's pin number
 * @return the value of wasPressed
 */
bool wasPressed(pin_t button);

/**
 * @brief function that goes through all elements in the buttons array and when it finds the button that was passed as an argument
 * 		 return the value of wasReleased for the button and if it was released -> change the variable was release to false (restore the value)
 * @param button, button's pin number
 * @return the value of wasReleased
 */
bool wasReleased(pin_t button);

/**
 * @brief function that goes through all elements in the buttons array and when it finds the button that was passed as an argument
 * 		 return the value of wasLkp for the button and if it was long key press -> change the variable was release to false (restore the value)
 * @param button, button's pin number
 * @return the value of wasLkp
 */
bool wasLkp(pin_t button);

/**
 * @brief Configure button array based on user input
 * @param button, button's pin number
 * @param type, button's type of working (typematic, lkp)
 * @return Configure succeed false if there was an error
 */
bool buttonConfiguration(pin_t button, int type,int time);

/**
 * @brief function that goes through all elements in the buttons array and when it finds the button that was passed as an argument
 * 		 return the value of wasTap for the button
 * @param button, button's pin number
 * @return the value of wasTap
 */
bool wasTap(pin_t button);


#endif /* BUTTON_H_ */
