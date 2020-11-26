/*
 * SensorsPosition.c
 *
 *  Created on: 23 Nov 2020
 *      Author: Grupo 2 - Labo de Micros
 */
/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "SensorsPosition.h"
#include "AccelMagn_drv.h"
#include "timer.h"
#include <math.h>
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct
{
	int16_t currentValue, previousValue;
	bool valueChanged;
}SensorsPosition_AngleData_t;



/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
SRAWDATA accelerometerCoordinates = {.x = -1, .y = -1, .z = -1};
SRAWDATA magnetometerCoordinates = {.x = 0, .y = -1, .z = 0}; //

SensorsPosition_AngleData_t roll ={.currentValue = 0, .previousValue = 0, .valueChanged = false};
SensorsPosition_AngleData_t pitch ={.currentValue = 0, .previousValue = 0, .valueChanged = false};
SensorsPosition_AngleData_t yaw ={.currentValue = 0, .previousValue = 0, .valueChanged = false};


static read_data sensorsData;
static bool isReading = false;

void (*_onAngleChangedCallback)(void);


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void Position_CalculateRoll(void);
static void Position_CalculatePitch(void);
static void Position_CalculateYaw(void);
static void callback_updatePos (void);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void SensorsPosition_Init(void (*onAngleChangedCallback)(void)){
	AccelMagn_init();
	_onAngleChangedCallback = onAngleChangedCallback;
	sensorsData.callback = callback_updatePos;
	sensorsData.pAccelData = &accelerometerCoordinates;
	sensorsData.pMagnData = &magnetometerCoordinates;
}

static void callback_updatePos (void)
{
	if (sensorsData.error != I2C_OK)
	{
		isReading = false;
		return;
	}
	Position_CalculateRoll();
	Position_CalculatePitch();
	Position_CalculateYaw();
}

SensorsPosition_EulerAngles_t SensorsPosition_GetEulerAngles(void)
{
	SensorsPosition_EulerAngles_t angles = {.roll = roll.currentValue, .yaw = yaw.currentValue, .pitch = pitch.currentValue};
	return angles;
}

void SensorsPosition_ReadData(void)
{
	if (isReading)
		return;
	isReading = true;
	AccelMagn_getData(&sensorsData);
}

int16_t SensorsPosition_GetRollAngle(void)
{
	return roll.currentValue;
}

int16_t SensorsPosition_GetPitchAngle(void)
{
	return pitch.currentValue;
}

int16_t SensorsPosition_GetYawAngle(void)
{
	return yaw.currentValue;
}

SensorsPosition_Angles_t SensorsPosition_GetChangedAngle(void)
{
	if(roll.valueChanged)
	{
		roll.valueChanged = false;
		return ROLL;
	}
	else if(pitch.valueChanged)
	{
		pitch.valueChanged = false;
		return PITCH;
	}
	else if(yaw.valueChanged)
	{
		yaw.valueChanged = false;
		return YAW;
	}
	return -1;
}

static void Position_CalculateRoll(void)
{
	roll.previousValue = roll.currentValue;
	if (accelerometerCoordinates.x != 0)
		roll.currentValue = (int)180*atan((float)accelerometerCoordinates.z/ (float)accelerometerCoordinates.x)/M_PI;
	else
		roll.currentValue = 90;


	int diference = fabsf((fabsf(roll.previousValue) - fabsf(roll.currentValue)));

	if(diference > 5)
	{
		roll.valueChanged = true;
		_onAngleChangedCallback();
	}
}

static void Position_CalculatePitch(void)
{
	pitch.previousValue = pitch.currentValue;
	if (accelerometerCoordinates.y != 0)
		pitch.currentValue = (int)180*atan((float)accelerometerCoordinates.z/ (float)accelerometerCoordinates.y)/M_PI;
	else
		pitch.currentValue = 90;

	int diference = fabsf(fabsf(pitch.previousValue) - fabsf(pitch.currentValue));

	if(diference > 5)
	{
		pitch.valueChanged = true;
		_onAngleChangedCallback();
	}
}

static void Position_CalculateYaw(void)
{
	yaw.previousValue = yaw.currentValue;
	if (magnetometerCoordinates.y != 0)
		yaw.currentValue =  (int)180*atan((float)magnetometerCoordinates.y/ (float)magnetometerCoordinates.x)/M_PI;
	else
		yaw.currentValue = 90;

	int diference = fabsf(fabsf(yaw.previousValue) - fabsf(yaw.currentValue));

	if(diference > 5)
	{
		yaw.valueChanged = true;
		_onAngleChangedCallback();
	}
}

/*******************************************************************************
 ******************************************************************************/
