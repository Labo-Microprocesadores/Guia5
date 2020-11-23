


#ifndef ACCELMAGN_H_
#define ACCELMAGN_H_

#include <hardware.h>
#include <stdint.h>

// the actual I2C address may be 0x1C, 0x1D, 0x1E or 0x1F

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
} SRAWDATA;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/





#endif /* ACCELMAGN_H_ */

