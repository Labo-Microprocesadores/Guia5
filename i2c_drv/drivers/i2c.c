/*
 * i2c.c
 *
 *  Created on: 13 nov. 2020
 *      Author: santi
 */

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "i2c.h"
#include "board.h"
#include "MK64F12.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define I2C_START_SIGNAL (i2c->C1 |= I2C_C1_MST_MASK)		  //generates start signal
#define I2C_STOP_SIGNAL (i2c->C1 &= ~I2C_C1_MST_MASK)		  //generetes stop signal
#define I2C_REPEAT_START_SIGNAL (i2c->C1 |= I2C_C1_RSTA_MASK) //generetes repeated start signal
#define I2C_WRITE_BYTE = (data) (i2c->D = data)				  //Write data for transfer
#define I2C_READ_BYTE (i2c->D)
#define I2C_GET_IRQ_FLAG (i2c->S & I2C_S_IICIF_MASK)
#define I2C_CLEAR_IRQ_FLAG (i2c->S |= I2C_S_IICIF_MASK)
#define I2C_GET_RX_ACK (i2c->S & I2C_S_RXAK_MASK)
#define I2C_SET_RX_MODE (i2c->C1 &= ~I2C_C1_TX_MASK)
#define I2C_SET_TX_MODE (i2c->C1 |= I2C_C1_TX_MASK)
#define I2C_SET_NACK (i2c->C1 |= I2C_C1_TXAK_MASK)
#define I2C_CLEAR_NACK (i2c->C1 &= ~I2C_C1_TXAK_MASK)
#define I2C_CHECK_BUS (i2c->S & I2C_S_BUSY_MASK)
#define I2C_GET_TCF (i2c->S & I2C_S_TCF_MASK)

#define BUS_BUSY 1
#define TIMEOUT 100

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

static I2C_Type *i2cPointers[] = I2C_BASE_PTRS;
static uint32_t simMasks[] = {SIM_SCGC4_I2C0_MASK, SIM_SCGC4_I2C1_MASK, SIM_SCGC1_I2C2_MASK};
static IRQn_Type i2cIRQs[] = {I2C0_IRQn, I2C1_IRQn, I2C2_IRQn }};
static I2C_Type *i2c;

static I2C_com_control_t *i2cCom;

static I2C_channel ch;
static uint8_t readDeviceAddress;
static uint8_t writeDeviceAddress;
static I2C_stage stage;
static I2C_mode mode;
static uint8_t dataIndex;

static void finish_com(I2C_fault fault);

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void I2C0_IRQHandler(void)
{
	i2c->S |= I2C_S_IICIF_MASK;	//Clear IRQ Flag
	uint8_t dummyData;

	switch (mode)
	{
	case (I2C_MODE_READ):
	{
		switch (stage)
		{
		case I2C_STAGE_WRITE_REG_ADDRESS:
		{
			if (i2c->S & I2C_S_RXAK_MASK == 0) // me llego un ACK
			{
				stage = I2C_STAGE_WRITE_DEV_ADDRESS_R;
				i2c->D = (i2cCom->register_address);
			}
			else
			{
				finish_com(I2C_SLAVE_ERROR);
			}
			break;
		}
		case I2C_STAGE_WRITE_DEV_ADDRESS_R:
		{
			if (i2c->S & I2C_S_RXAK_MASK == 0) // me llego un ACK
			{
				stage = I2C_STAGE_READ_DUMMY_DATA;
				i2c->C1 |= I2C_C1_RSTA_MASK;
				i2c->D = (readDeviceAddress);
			}
			else
			{
				finish_com(I2C_SLAVE_ERROR);
			}
			break;
		}
		case I2C_STAGE_READ_DUMMY_DATA:
		{

			if (i2c->S & I2C_S_RXAK_MASK == 0) //ACK reveived
			{
				stage = I2C_STAGE_READ_DATA;
				i2c->C1 &= ~I2C_C1_TX_MASK;	//Set RX mode

				if (dataIndex == i2cCom->data_size - 1) //Reading last data
				{
					i2c->C1 |= I2C_C1_TXAK_MASK;
				}

				dummyData = i2c->D;
			}
			else
			{
				finish_com(I2C_SLAVE_ERROR);
			}
			break;
		}
		case I2C_STAGE_READ_DATA:
		{
			if (dataIndex == i2cCom->data_size - 1)
			{

				finish_com(I2C_NO_FAULT);
			}
			else
			{
				if (dataIndex == i2cCom->data_size - 2) //voy a leer mi último dato
				{
					i2c->C1 |= I2C_C1_TXAK_MASK;
				}

				i2cCom->data[dataIndex] = i2c->D;
				dataIndex++;
			}

			break;
		}
		default:
			break;
		}
		break;
	}
	case I2C_MODE_WRITE:
	{
		switch (stage)
		{
		case I2C_STAGE_WRITE_REG_ADDRESS:
		{
			if (i2c->S & I2C_S_RXAK_MASK == 0) // me llego un ACK
			{
				i2c->D = (i2cCom->register_address);
				stage = I2C_STAGE_WRITE_DATA;
			}
			else
			{
				finish_com(I2C_SLAVE_ERROR);
			}
			break;
		}
		case I2C_STAGE_WRITE_DATA:
		{
			if (i2c->S & I2C_S_RXAK_MASK == 0) // me llego un ACK
			{
				if (dataIndex == i2cCom->data_size)
				{
					finish_com(I2C_NO_FAULT);
				}
				else
				{
					i2c->D = (i2cCom->data[dataIndex]);
					dataIndex++;
				}
			}
			else
			{
				finish_com(I2C_SLAVE_ERROR);
			}
		}
		default:
			break;
		}
	}
	break;
	}
}

void finish_com(I2C_fault fault)
{
	i2c->C1 &= ~I2C_C1_MST_MASK;
	i2cCom->fault = fault;

	if ((mode == I2C_MODE_READ) & (fault == I2C_NO_FAULT))
	{
		i2c->C1 &= ~I2C_C1_TXAK_MASK;	//Clear nack
		i2cCom->data[dataIndex] = i2c->D;
	}

	stage = I2C_STAGE_NONE;
	i2cCom->callback();
}

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void i2cInit(I2C_channel channel)
{
	SIM_Type *sim_ptr = SIM;
	PORT_Type *portsPtrs[] = PORT_BASE_PTRS;
	i2c = i2cPointers[channel];
	ch = channel;

	PORT_Type *port_SDA = portsPtrs[PIN2PORT(I2C_SDA)];
	uint32_t pin_SDA = PIN2NUM(I2C_SDA);

	PORT_Type *port_SCL = portsPtrs[PIN2PORT(I2C_SCL)];
	uint32_t pin_SCL = PIN2NUM(I2C_SCL);

	sim_ptr->SCGC5 |= SIM_SCGC5_PORTE_MASK;

	if (channel == I2C_2)
	{
		sim_ptr->SCGC1 |= simMasks[channel]; // activo clock gating
	}
	else
	{
		sim_ptr->SCGC4 |= simMasks[channel]; // activo clock gating
	}

	i2c->C1 = 0x00;				  // I2C Control Register 1
	i2c->C1 |= I2C_C1_IICEN_MASK; // Enables I2C module operation.
	i2c->C1 |= I2C_C1_IICIE_MASK; // Enables I2C interrupt requests.
	i2c->S = I2C_S_TCF_MASK | I2C_S_IICIF_MASK;

	// I2C Frequency Divider register

	//I2C baud rate = I2C module clock speed (Hz)/(mul × SCL divider)
	i2c->F = I2C_F_MULT(0) | I2C_F_ICR(0); //  set the I2C baud rate

	NVIC_EnableIRQ(i2cIRQs[channel]);

	port_SDA->PCR[pin_SDA] |= PORT_PCR_MUX(5); // cambia los pines a alternativa i2c
	port_SDA->PCR[pin_SDA] |= PORT_PCR_ODE_MASK;
	port_SCL->PCR[pin_SCL] |= PORT_PCR_MUX(5);
	port_SCL->PCR[pin_SCL] |= PORT_PCR_ODE_MASK;
}

void i2cReadMsg(I2C_com_control_t *i2cComm)
{

	if (i2c->S & I2C_S_BUSY_MASK != BUS_BUSY)
	{
		i2cCom = i2cComm;

		readDeviceAddress = (i2cCom->slave_address << 1) | 0b00000001;
		writeDeviceAddress = (i2cCom->slave_address << 1) & 0b11111110;
		dataIndex = 0;

		i2cCom->fault = I2C_NO_FAULT;

		stage = I2C_STAGE_WRITE_REG_ADDRESS;
		mode = I2C_MODE_READ;

		i2c->C1 |= I2C_C1_TX_MASK;	// Transmit Mode Select (TRANSMIT)
		i2c->C1 |= I2C_C1_MST_MASK; // Master Mode Select (MASTER)
		i2c->D = writeDeviceAddress;
	}
	else
	{
		i2cComm->fault = I2C_BUS_BUSY;
		i2cComm->callback();
	}

	return;
}

void i2cWriteMsg(I2C_com_control_t *i2cComm)
{

	if (i2c->S & I2C_S_BUSY_MASK != BUS_BUSY)
	{
		i2cCom = i2cComm;

		readDeviceAddress = (i2cCom->slave_address << 1) | 0b00000001;
		writeDeviceAddress = (i2cCom->slave_address << 1) & 0b11111110;
		dataIndex = 0;

		i2cCom->fault = I2C_NO_FAULT;

		stage = I2C_STAGE_WRITE_REG_ADDRESS;
		mode = I2C_MODE_WRITE;

		i2c->C1 |= I2C_C1_TX_MASK;	// Transmit Mode Select (TRANSMIT)
		i2c->C1 |= I2C_C1_MST_MASK; // Master Mode Select (MASTER) //START signal
		i2c->D = writeDeviceAddress;
	}
	else
	{
		i2cComm->fault = I2C_BUS_BUSY;
		i2cComm->callback();
	}
	return;
}

