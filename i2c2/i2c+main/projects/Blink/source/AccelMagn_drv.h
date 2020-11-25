/***************************************************************************//**
  @file     AccelMagn.h
  @brief    Accelerometer and Magnetometer driver
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/

#ifndef ACCELMAGN_H_
#define ACCELMAGN_H_

#include <hardware.h>
#include <stdint.h>


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
} SRAWDATA;

typedef enum {I2C_ERROR, I2C_OK} I2C_STATUS;

typedef void (*callbackp)(void);

typedef struct {
	SRAWDATA * pMagnData;
	SRAWDATA * pAccelData;
	callbackp callback;
	I2C_STATUS error;
} read_data;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Accelerometer and magnetometer initialization.
 * @param Read data's pointer
 * @return I2C_OK if I2C was initialized correctly.
 * 		   I2C_ERRPR if an error occured while trying to initialize I2C.
 *
 * This function configures the FXOS8700CQ for 200-Hz hybrid mode meaning that
 * both accelerometer and magnetometer data are provided at the 200-Hz rate.
 * This function configures FXOS8700CQ combination accelerometer and magnetometer sensor
 * https://www.nxp.com/docs/en/data-sheet/FXOS8700CQ.pdf
 */
I2C_STATUS AccelMagn_init(void);

/**
 * @brief Gets last data from the accelerometer and magnetometer.
 * @param Read data's pointer
 * @return ---
 *
 * https://www.nxp.com/docs/en/data-sheet/FXOS8700CQ.pdf
 */
void AccelMagn_getData(read_data * data);



#endif /* ACCELMAGN_H_ */

