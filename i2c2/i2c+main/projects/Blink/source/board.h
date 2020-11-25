/***************************************************************************//**
  @file     board.h
  @brief    Board management
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/

#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <gpio.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/***** BOARD defines **********************************************************/

// On Board User LEDs
#define PIN_LED_RED     PORTNUM2PIN(PB,22) // PTB22
#define PIN_LED_GREEN   PORTNUM2PIN(PE,26) // PTE26
#define PIN_LED_BLUE    PORTNUM2PIN(PB,21) // PTB21

#define TEST_PIN		PORTNUM2PIN(PE,24)

#define LED_ACTIVE      LOW


// On Board User Switches
#define PIN_SW2         PORTNUM2PIN(PC,6) // PTC6
#define PIN_SW3         PORTNUM2PIN(PA,4) // PTA4

#define SW_ACTIVE       LOW
#define SW_INPUT_TYPE   INPUT_PULLDOWN

//I2C pins
#define I2C_SDA			PORTNUM2PIN(PE,25)
#define I2C_SCL			PORTNUM2PIN(PE,24)

#define I2C0_SDA		PORTNUM2PIN(PE,25)
#define I2C0_SCL		PORTNUM2PIN(PE,24)
#define I2C1_SDA		PORTNUM2PIN(PC,11)
#define I2C1_SCL		PORTNUM2PIN(PC,10)
#define I2C2_SDA		PORTNUM2PIN(PA,13)
#define I2C2_SCL		PORTNUM2PIN(PA,14)
/*******************************************************************************
 ******************************************************************************/

#endif // _BOARD_H_
