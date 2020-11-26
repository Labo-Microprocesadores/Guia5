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

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes drivers for roll, pitching and yaw measures.
 */
void SensorsPosition_Init(void (*funcallback)(void));

/**
 * @brief updates sensors internal data. (PASAR COMO CALLBACK)
 */
void SensorsPosition_ReadAccelerometer(void);

/**
 * @brief calculates the new rolling angle from the last aceleration data.
 * @return roll angle.
 */
int16_t SensorsPosition_GetRollAngle(void);

/**
 * @brief calculates the new pitching angle from the last aceleration data.
 * @return pitching angle.
 */
int16_t SensorsPosition_GetPitchAngle(void);

/**
 * @brief calculates the new yaw from the last magnetometer data.
 * @return yaw angle.
 */
int16_t SensorsPosition_GetYawAngle(void);

/**
 * @brief gets the parameter that changed.
 * @return parameter_EVENT.
 */
SensorsPosition_Angles_t SensorsPosition_GetChangedAngle(void);

#endif /* SENSORS_POSITION_H_ */
