/*
 * i2c.h
 *
 *  Created on: Sep 16, 2019
 *      Author: martinamaspero
 */

#ifndef I2C_H_
#define I2C_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>




/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



#define ADDRESS_CYCLE_BYTES 2

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef void (* pfunc) (void);
typedef enum {I2C_0, I2C_1, I2C_2}I2C_ChannelType;


typedef enum
{
         I2C_STAGE_NONE = 0,
         I2C_STAGE_WRITE_DATA,
         I2C_STAGE_WRITE_DEV_ADDRESS_W,
         I2C_STAGE_WRITE_DEV_ADDRESS_R,
         I2C_STAGE_WRITE_REG_ADDRESS,
         I2C_STAGE_READ_DUMMY_DATA,
         I2C_STAGE_READ_DATA,


} I2C_STAGE;

typedef enum
{
         I2C_MODE_READ = 0,
         I2C_MODE_WRITE,
} I2C_MODE;



typedef enum
{
         I2C_NO_FAULT = 0,
         I2C_BUS_BUSY,
         I2C_TIMEOUT,
         I2C_SLAVE_ERROR,
} I2C_FAULT;


typedef struct
{
	uint8_t * data;
	uint8_t data_size; // en bytes
	uint8_t register_address;
	uint8_t slave_address;
	pfunc callback;
	I2C_FAULT fault;

}I2C_COM_CONTROL;


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize i2c driver
 * @param id i2c's number
 * @param config i2c's configuration (baudrate, parity, etc.)
*/
void i2cInit (uint8_t channel); //, uint8_t baud_rate, uint32_t systemClock


/**
 * @brief Read a received message. Non-Blocking
 * @param id i2c's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
*/
void i2cReadMsg(I2C_COM_CONTROL * i2c_comm);


/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id i2c's number
 * @param msg Buffer with the bytes to be transfered
 * @param cant Desired quantity of bytes to be transfered
 * @return Real quantity of bytes to be transfered
*/
void i2cWriteMsg(I2C_COM_CONTROL * i2c_comm);


I2C_FAULT i2cReadMsgBlocking (uint8_t * buffer, uint8_t data_size,	uint8_t register_address, uint8_t slave_address );

I2C_FAULT i2cWriteMsgBlocking (uint8_t * msg, uint8_t data_size,	uint8_t register_address, uint8_t slave_address );


/*******************************************************************************
 ******************************************************************************/


#endif /* I2C_H_ */
