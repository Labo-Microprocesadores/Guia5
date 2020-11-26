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
enum{CLOCKWISE, COUNTER_CLOCKWISE, ZERO};


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
SRAWDATA accelerometerCoordinates = {.x = -1, .y = -1, .z = -1};
SRAWDATA magentometerCoordinates = {.x = 0, .y = -1, .z = 0}; //

int16_t roll = 0; 		// Form -179° to 180°
int16_t previousRoll = 0;
bool rollChanged = false;

int16_t pitch = 0;	// Form -179° to 180°
int16_t previousPitch = 0;
bool pitchChanged = false;

int16_t yaw = 0;	// Form -179° to 180°
int16_t previousYaw = 0;
bool yawChanged = false;

static read_data accelerometerData;
static bool isReading = false;

void (*__onAngleChangedCallback)(void);


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void Position_CalculateRoll(void);
static void Position_CalculatePitching(void);
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
	accelerometerData.callback = callback_updatePos;
	accelerometerData.pAccelData = &accelerometerCoordinates;
	accelerometerData.pMagnData = &magentometerCoordinates;
}

static void callback_updatePos (void)
{
	if (!accelerometerData.error == I2C_OK)
	{
		isReading = false;
		return;
	}
	Position_CalculateRoll();
	Position_CalculatePitching();
	Position_CalculateYaw();
}

void SensorsPosition_ReadAccelerometer(void)
{
	if (isReading)
		return;
	AccelMagn_getData(&accelerometerData);

}

int16_t SensorsPosition_GetRollAngle(void)
{
	return roll;
}

int16_t SensorsPosition_GetPitchAngle(void)
{
	return pitch;
}

int16_t SensorsPosition_GetYawAngle(void)
{
	return yaw;
}

SensorsPosition_Angles_t SensorsPosition_GetChangedAngle(void)
{
	if(rollChanged)
	{
		rollChanged = false;
		return ROLL;
	}
	else if(pitchChanged)
	{
		pitchChanged = false;
		return PITCH;
	}
	else if(yawChanged)
	{
		yawChanged = false;
		return YAW;
	}
}

static void Position_CalculateRoll(void)
{
	uint8_t clock_type;
	int theta;

	if (accelerometerCoordinates.x != 0)
		theta = (int)180*atan((float)accelerometerCoordinates.z/ (float)accelerometerCoordinates.x)/M_PI;
	else
		theta = 90;


	/*if(accelerometerCoordinates.x < 0){
		clock_type = CLOCKWISE;
	}else if(accelerometerCoordinates.x > 0){
		clock_type = COUNTER_CLOCKWISE;
	}else{
		clock_type = ZERO;
	}

	if(clock_type != ZERO){

		theta = (int)180*atan((float)accelerometerCoordinates.z/ (float)accelerometerCoordinates.x)/M_PI;

	}else{
		theta = 90;
	}*/

	switch (clock_type) {
		case CLOCKWISE:
			if(accelerometerCoordinates.z != 0){
				previousRoll = - 90 - theta;
			}else{
				previousRoll = - 90;
			}
			break;
		case COUNTER_CLOCKWISE:
			if(accelerometerCoordinates.z != 0){
				previousRoll = 90 - theta;
			}else{
				previousRoll = 90;
			}
			break;

		case ZERO:
			if(accelerometerCoordinates.z > 0){
				previousRoll = 0;
			}else if(accelerometerCoordinates.z < 0){
				previousRoll = 180;
			}
			break;
	}

	int diference = fabsf((fabsf(previousRoll) - fabsf(roll)));

	if(diference > 5) // Condicion de consigna para tomar nuevo cambio
	{
		roll = previousRoll;
		rollChanged = true;
		_onAngleChangedCallback(); // Aviso a App que cambio el angulo mas de 5°
	}
}

static void Position_CalculatePitching(void)
{
	uint8_t clock_type;
	int theta;

	if(accelerometerCoordinates.y < 0){
		clock_type = CLOCKWISE;
	}else if(accelerometerCoordinates.y > 0){
		clock_type = COUNTER_CLOCKWISE;
	}else{
		clock_type = ZERO;
	}

	if(clock_type != ZERO){

		theta = (int)180*atan((float)accelerometerCoordinates.z/ (float)accelerometerCoordinates.y)/M_PI;

	}else{
		theta = 90;
	}

	switch (clock_type) {
		case CLOCKWISE:
			if(accelerometerCoordinates.z != 0){
				previousPitch = - 90 - theta;
			}else{
				previousPitch = - 90;
			}
			break;
		case COUNTER_CLOCKWISE:
			if(accelerometerCoordinates.z != 0){
				previousPitch = 90 - theta;
			}else{
				previousPitch = 90;
			}
			break;
		case ZERO:
			if(accelerometerCoordinates.z < 0){
				previousPitch = 0;
			}else if(accelerometerCoordinates.z > 0){
				previousPitch = 180; // Por defecto, podria ser tambien
			}
			break;
	}

	int diference = fabsf(fabsf(previousPitch) - fabsf(pitch));

	if(diference > 5)
	{
		pitch = previousPitch;
		pitchChanged = true;
		_onAngleChangedCallback(); // Aviso a App que cambio el angulo mas de 5°
	}
}

static void Position_CalculateYaw(void)
{

	uint8_t clock_type;
	int theta;

	if(magentometerCoordinates.y < 0){
		clock_type = CLOCKWISE;
	}else if(magentometerCoordinates.y > 0){
		clock_type = COUNTER_CLOCKWISE;
	}else{
		clock_type = ZERO;
	}

	if(clock_type != ZERO){

		theta = (int)180*atan((float)magentometerCoordinates.y/ (float)magentometerCoordinates.x)/M_PI;

	}else{
		theta = 90;
	}

	switch (clock_type) {
		case CLOCKWISE:
			if(magentometerCoordinates.x > 0){
				previousYaw = - 90 + theta;
			}else if(magentometerCoordinates.x < 0){
				previousYaw = 90 + theta;
			}else{
				previousYaw = 180;
			}
			break;
		case COUNTER_CLOCKWISE:
			if(magentometerCoordinates.x < 0){
				previousYaw = 90 + theta;
			}else if(magentometerCoordinates.x > 0){
				previousYaw = - 90 + theta;
			}else{
				previousYaw = 0;
			}
			break;
		case ZERO:
			if(magentometerCoordinates.x < 0){
				previousYaw = theta;
			}else if(magentometerCoordinates.x > 0){
				previousYaw = 0 - theta; // Por defecto, podria ser tambien
			}
			break;
	}


	int diference = fabsf(fabsf(previousYaw) - fabsf(yaw));

	if(diference > 5)
	{
		yaw = previousYaw;
		yawChanged = true;
		_onAngleChangedCallback(); // Aviso a App que cambio el angulo mas de 5°
	}
}

/*******************************************************************************
 ******************************************************************************/
