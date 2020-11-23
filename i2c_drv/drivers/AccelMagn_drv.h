


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

I2C_STATUS _mqx_ints_FXOS8700CQ_start(void);
//I2C_STATUS AccelMagn_init(void);
void AccelMagn_getData(read_data * data);



#endif /* ACCELMAGN_H_ */

