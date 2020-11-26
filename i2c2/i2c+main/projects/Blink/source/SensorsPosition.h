/*
 * SensorsPosition.h
 *
 *  Created on: 23 Nov 2020
 *      Author: Grupo 2 - Labo de Micros
 */

#ifndef SENSORS_POSITION_H_
#define SENSORS_POSITION_H_

#include <stdint.h>
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {ROLL, PITCH, YAW} SensorsPosition_Angles_t;
typedef struct
{
	int16_t roll, pitch, yaw;
}SensorsPosition_EulerAngles_t;
/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes the driver.
 * param onAngleChangedCallback callback to be called when an angle changes. A change is determined by a difference of at least 5 degrees from the last measurement.
 */
void SensorsPosition_Init(void (*onAngleChangedCallback)(void));

/**
 * @brief Get roll, pitch and yaw angles.
 */
SensorsPosition_EulerAngles_t SensorsPosition_GetEulerAngles(void);


/**
 * @brief Reads the data from magnetometer and accelerometer sensors.
 */
void SensorsPosition_ReadData(void);

/**
 * @brief Returns the "roll" angle.
 * @return roll angle. -180 to +180
 */
int16_t SensorsPosition_GetRollAngle(void);

/**
 * @brief Returns the "pitch" angle.
 * @return pitch angle. -90 to +90
 */
int16_t SensorsPosition_GetPitchAngle(void);

/**
 * @brief Returns the "yaw" angle.
 * @return yaw angle. -180 to +180
 */
int16_t SensorsPosition_GetYawAngle(void);

/**
 * @brief returns the angle that has changed. A change is determined by a difference of at least 5 degrees from the last measurement.
 * @return SensorsPosition_Angles_t angle.
 */
SensorsPosition_Angles_t SensorsPosition_GetChangedAngle(void);

#endif /* SENSORS_POSITION_H_ */
