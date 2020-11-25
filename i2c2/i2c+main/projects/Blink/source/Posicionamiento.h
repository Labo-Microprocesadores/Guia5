/*
 * Posicionamiento.h
 *
 *  Created on: 20 Sep 2019
 *      Author: Marcelo
 */

#ifndef POSICIONAMIENTO_H_
#define POSICIONAMIENTO_H_

#include <stdint.h>
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef int16_t roll_t;			// Rolido
typedef	int16_t pitching_t;		// Cabeceo
typedef int16_t orientation_t;	// Orientacion

#define ROLL_EVENT 1
#define PITCHING_EVENT 2
#define ORIENTATION_EVENT 3

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes drivers for roll, pitching and orientation measures.
 */
void Position_InitDrv(void (*funcallback)(void));

/**
 * @brief updates sensors internal data. (PASAR COMO CALLBACK)
 */
void Position_Update(void);

/**
 * @brief calculates the new rolling angle from the last aceleration data.
 * @return roll angle.
 */
roll_t Position_GetRoll(void);

/**
 * @brief calculates the new pitching angle from the last aceleration data.
 * @return pitching angle.
 */
pitching_t Position_GetPitch(void);

/**
 * @brief calculates the new orientation from the last magnetometer data.
 * @return orientation angle.
 */
orientation_t Position_GetOrientation(void);

/**
 * @brief gets the parameter that changed.
 * @return parameter_EVENT.
 */
int Position_GetChangeEvent(void);

#endif /* POSICIONAMIENTO_H_ */
