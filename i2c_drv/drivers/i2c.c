
/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "i2c.h"
#include "board.h"
#include "uart.h"
#include "MK64F12.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define I2C_START_SIGNAL         (i2c->C1 |= I2C_C1_MST_MASK) //generates start signal
#define I2C_STOP_SIGNAL          (i2c->C1 &= ~I2C_C1_MST_MASK)//generetes stop signal
#define I2C_REPEAT_START_SIGNAL  (i2c->C1 |= I2C_C1_RSTA_MASK) //generetes repeated start signal
#define I2C_WRITE_BYTE(data)     (i2c->D = data) //Write data for transfer
#define I2C_READ_BYTE            (i2c->D)
#define I2C_GET_IRQ_FLAG         (i2c->S & I2C_S_IICIF_MASK)
#define I2C_CLEAR_IRQ_FLAG       (i2c->S |= I2C_S_IICIF_MASK)
#define I2C_GET_RX_ACK			 (i2c->S & I2C_S_RXAK_MASK)
#define I2C_SET_RX_MODE          (i2c->C1 &= ~I2C_C1_TX_MASK)
#define I2C_SET_TX_MODE          (i2c->C1 |= I2C_C1_TX_MASK)
#define I2C_SET_NACK	         (i2c->C1 |= I2C_C1_TXAK_MASK)
#define I2C_CLEAR_NACK      	 (i2c->C1 &= ~I2C_C1_TXAK_MASK)
#define I2C_CHECK_BUS		  	 (i2c->S & I2C_S_BUSY_MASK)
#define I2C_GET_TCF				 (i2c->S & I2C_S_TCF_MASK)

#define BUS_BUSY 				 1
#define TIMEOUT					 100

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/


static I2C_Type* i2cPtrs [] = I2C_BASE_PTRS;
static uint32_t simMasks[] = {SIM_SCGC4_I2C0_MASK, SIM_SCGC4_I2C1_MASK, SIM_SCGC1_I2C2_MASK};
static IRQn_Type i2c_irqs[] = I2C_IRQS;
static I2C_Type* i2c;

static I2C_COM_CONTROL * i2c_com;

static I2C_ChannelType ch;
static uint8_t device_address_r;
static uint8_t device_address_w;
static I2C_STAGE stage;
static I2C_MODE mode;
static uint8_t data_index;

static void finish_com (I2C_FAULT fault);

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


void I2C0_IRQHandler(void)
{
	I2C_CLEAR_IRQ_FLAG;
	uint8_t dummy_data;
	switch(mode)
	{
		case(I2C_MODE_READ):
		{
			switch(stage)
			{
				case I2C_STAGE_WRITE_REG_ADDRESS:
				{
					if(I2C_GET_RX_ACK == 0)// me llego un ACK
					{
						stage = I2C_STAGE_WRITE_DEV_ADDRESS_R;
						I2C_WRITE_BYTE(i2c_com->register_address);
					}
					else
					{
						finish_com(I2C_SLAVE_ERROR);
					}
					break;
				}
				case I2C_STAGE_WRITE_DEV_ADDRESS_R:
				{
					if(I2C_GET_RX_ACK == 0)// me llego un ACK
					{
						stage = I2C_STAGE_READ_DUMMY_DATA;
						I2C_REPEAT_START_SIGNAL;
						I2C_WRITE_BYTE(device_address_r);

					}
					else
					{
						finish_com(I2C_SLAVE_ERROR);
					}
					break;
				}
				case I2C_STAGE_READ_DUMMY_DATA:
				{
					if(I2C_GET_RX_ACK == 0)// me llego un ACK
					{
						stage = I2C_STAGE_READ_DATA;
						I2C_SET_RX_MODE;
						if(data_index == i2c_com->data_size-1) //voy a leer mi último dato
						{
							I2C_SET_NACK;
						}
						dummy_data = I2C_READ_BYTE;
					}
					else
					{
						finish_com(I2C_SLAVE_ERROR);
					}
					break;
				}
				case I2C_STAGE_READ_DATA:
				{
					if(data_index == i2c_com->data_size-1)
					{
						finish_com(I2C_NO_FAULT);
					}
					else
					{
						if(data_index == i2c_com->data_size-2) //voy a leer mi último dato
						{
							I2C_SET_NACK;
						}
					i2c_com->data[data_index] = I2C_READ_BYTE;
					data_index++;

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
			switch(stage)
			{
				case I2C_STAGE_WRITE_REG_ADDRESS:
				{
					if(I2C_GET_RX_ACK == 0)// me llego un ACK
					{
						I2C_WRITE_BYTE(i2c_com->register_address);
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
					if(I2C_GET_RX_ACK == 0)// me llego un ACK
					{
						if(data_index == i2c_com->data_size)
						{
							finish_com(I2C_NO_FAULT);
						}
						else
						{
							I2C_WRITE_BYTE(i2c_com->data[data_index]);
							data_index++;
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


void finish_com (I2C_FAULT fault)
{
	I2C_STOP_SIGNAL;
	i2c_com->fault = fault;

	if ((mode == I2C_MODE_READ) & (fault == I2C_NO_FAULT))
	{
		I2C_CLEAR_NACK;
		i2c_com->data[data_index] = I2C_READ_BYTE;
	}

	stage = I2C_STAGE_NONE;
	i2c_com->callback();

}



/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/




void i2cInit (I2C_ChannelType channel)
{
	SIM_Type* sim_ptr = SIM;
	PORT_Type * portsPtrs [] = PORT_BASE_PTRS;
	i2c = i2cPtrs[channel];
	ch = channel;

	PORT_Type * port_SDA = portsPtrs[PIN2PORT(I2C_SDA)];
	uint32_t pin_SDA = PIN2NUM(I2C_SDA);

	PORT_Type * port_SCL = portsPtrs[PIN2PORT(I2C_SCL)];
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


	 i2c->C1 = 0x00; // I2C Control Register 1
	 i2c->C1 |= I2C_C1_IICEN_MASK; // Enables I2C module operation.
	 i2c->C1 |= I2C_C1_IICIE_MASK; // Enables I2C interrupt requests.
	 i2c->S = I2C_S_TCF_MASK | I2C_S_IICIF_MASK;


	 // I2C Frequency Divider register

	 //I2C baud rate = I2C module clock speed (Hz)/(mul × SCL divider)
	 i2c->F = I2C_F_MULT(0) | I2C_F_ICR(0); //  set the I2C baud rate

	 NVIC_EnableIRQ(i2c_irqs[channel]);


	 port_SDA->PCR[pin_SDA] |= PORT_PCR_MUX(5); // cambia los pines a alternativa i2c
	 port_SDA->PCR[pin_SDA] |= PORT_PCR_ODE_MASK;
	 port_SCL->PCR[pin_SCL] |= PORT_PCR_MUX(5);
	 port_SCL->PCR[pin_SCL] |= PORT_PCR_ODE_MASK;
}


void i2cReadMsg(I2C_COM_CONTROL * i2c_comm)
{


	if(I2C_CHECK_BUS != BUS_BUSY)
	{
		i2c_com = i2c_comm;


		device_address_r = (i2c_com->slave_address << 1) | 0b00000001;
		device_address_w = (i2c_com->slave_address << 1) & 0b11111110;
		data_index = 0;

		i2c_com->fault = I2C_NO_FAULT;


		 stage =  I2C_STAGE_WRITE_REG_ADDRESS;
		 mode =  I2C_MODE_READ;


		 i2c->C1 |= I2C_C1_TX_MASK; // Transmit Mode Select (TRANSMIT)
		 i2c->C1 |= I2C_C1_MST_MASK; // Master Mode Select (MASTER)
		 I2C_WRITE_BYTE(device_address_w);

	}
	else
	{
		i2c_comm->fault = I2C_BUS_BUSY;
		i2c_comm->callback();
	}

	return;
}


void i2cWriteMsg(I2C_COM_CONTROL * i2c_comm)
{

	if(I2C_CHECK_BUS != BUS_BUSY)
	{
		i2c_com = i2c_comm;

		device_address_r = (i2c_com->slave_address << 1) | 0b00000001;
		device_address_w = (i2c_com->slave_address << 1) & 0b11111110;
		data_index = 0;

		i2c_com->fault = I2C_NO_FAULT;


		 stage =  I2C_STAGE_WRITE_REG_ADDRESS;
		 mode =  I2C_MODE_WRITE;


		 i2c->C1 |= I2C_C1_TX_MASK; // Transmit Mode Select (TRANSMIT)
		 i2c->C1 |= I2C_C1_MST_MASK; // Master Mode Select (MASTER) //START signal
		 I2C_WRITE_BYTE(device_address_w);


	}
	else
	{
		i2c_comm->fault = I2C_BUS_BUSY;
		i2c_comm->callback();
	}
	return;

}




I2C_FAULT i2cWriteMsgBlocking (uint8_t * msg, uint8_t data_size,	uint8_t register_address, uint8_t slave_address )
{
	NVIC_DisableIRQ(i2c_irqs[ch]);

	I2C_FAULT fault = I2C_NO_FAULT;

	unsigned timeout = TIMEOUT;	// Use timeout for blocking communications

	if(I2C_CHECK_BUS != BUS_BUSY)
	{
		device_address_w = (slave_address << 1) & 0b11111110;
		data_index = 0;

		i2c->C1 |= I2C_C1_TX_MASK; // Transmit Mode Select (TRANSMIT)
		i2c->C1 |= I2C_C1_MST_MASK; // Master Mode Select (MASTER)
		I2C_WRITE_BYTE(device_address_w);

		while(I2C_GET_TCF == false)
		{
			timeout--;
			if(timeout == 0)
			{
				I2C_STOP_SIGNAL;
				fault = I2C_TIMEOUT;
			}
		}

		if(I2C_GET_RX_ACK == 0 && fault == I2C_NO_FAULT)
		{

			I2C_WRITE_BYTE(register_address);

			while( I2C_GET_TCF == false )
			{
				timeout--;
				if(timeout == 0)
				{
					I2C_STOP_SIGNAL;
					fault = I2C_TIMEOUT;
				}
			}

			while((data_index <= data_size) && (fault == I2C_NO_FAULT))
			{
				if(I2C_GET_RX_ACK == 0 && fault == I2C_NO_FAULT)
				{
					if(data_index == data_size)
					{
						I2C_STOP_SIGNAL;
					}
					else
					{
						I2C_WRITE_BYTE(msg[data_index]);


						while(I2C_GET_TCF == false)
						{
							timeout--;
							if(timeout == 0)
							{
								fault = I2C_TIMEOUT;
							}
						}
					}

					data_index++;
				}
				else
				{
					I2C_STOP_SIGNAL;
					fault = I2C_SLAVE_ERROR;
				}

			}

		}
		else
		{
			I2C_STOP_SIGNAL;
			fault =  I2C_SLAVE_ERROR;
		}

	}
	else
	{
		return I2C_BUS_BUSY;
	}

	while(I2C_CHECK_BUS == BUS_BUSY)
	{
		//STOP
	};

	NVIC_EnableIRQ(i2c_irqs[ch]);
	return I2C_NO_FAULT;

}




I2C_FAULT i2cReadMsgBlocking (uint8_t * buffer, uint8_t data_size,	uint8_t register_address, uint8_t slave_address )
{
	NVIC_DisableIRQ(i2c_irqs[ch]);

	I2C_FAULT fault = I2C_NO_FAULT;

	unsigned timeout = TIMEOUT;	// Use timeout for blocking communications

	if(I2C_CHECK_BUS != BUS_BUSY)
	{

		device_address_r = (slave_address << 1) | 0b00000001;
		device_address_w = (slave_address << 1) & 0b11111110;
		data_index = 0;

		i2c->C1 |= I2C_C1_TX_MASK; // Transmit Mode Select (TRANSMIT)
		i2c->C1 |= I2C_C1_MST_MASK; // Master Mode Select (MASTER)
		I2C_WRITE_BYTE(device_address_w);

		while(I2C_GET_TCF == false)
		{
			timeout--;
			if(timeout == 0)
			{
				I2C_STOP_SIGNAL;
				fault = I2C_TIMEOUT;
			}
		}

		if(I2C_GET_RX_ACK == 0 && fault == I2C_NO_FAULT)
		{

			I2C_WRITE_BYTE(register_address);

			while( I2C_GET_TCF == false )
			{
				timeout--;
				if(timeout == 0)
				{
					I2C_STOP_SIGNAL;
					fault = I2C_TIMEOUT;
				}
			}
			if((I2C_GET_RX_ACK) == 0 && fault == I2C_NO_FAULT)
			{

				I2C_REPEAT_START_SIGNAL;
				I2C_WRITE_BYTE(device_address_r);
				while( I2C_GET_TCF == false)
				{
					timeout--;
					if(timeout == 0)
					{
						I2C_STOP_SIGNAL;
						fault = I2C_TIMEOUT;
					}
				}
				if(I2C_GET_RX_ACK == 0 && fault == I2C_NO_FAULT)
				{
					for (int i = 100; i>0; i--){};

					I2C_SET_RX_MODE;

					if(data_index == data_size-1) //voy a leer mi último dato
					{
						I2C_SET_NACK;
					}
					uint8_t dummy_data = I2C_READ_BYTE;

					while( I2C_GET_TCF == false )
					{
						timeout--;
						if(timeout == 0)
						{
							I2C_STOP_SIGNAL;
							fault = I2C_TIMEOUT;
						}

					}

					if(fault == I2C_NO_FAULT)
					{
						while(data_index < data_size)
						{
							if(data_index == data_size-1)
							{
								I2C_STOP_SIGNAL;
								I2C_CLEAR_NACK;
								buffer[data_index] = I2C_READ_BYTE;
							}

							else
							{
								if(data_index == data_size-2) //voy a leer mi último dato
								{
									I2C_SET_NACK;
								}

								buffer[data_index] = I2C_READ_BYTE;
								while(I2C_GET_TCF == false)
								{
									timeout--;
									if(timeout == 0)
									{
										return I2C_TIMEOUT;
									}
								}
							}

							data_index++;
						}
					}


				}
				else
				{
					I2C_STOP_SIGNAL;
					fault = I2C_SLAVE_ERROR;
				}


			}

			else
			{
				I2C_STOP_SIGNAL;
				fault = I2C_SLAVE_ERROR;
			}
		}
		else
		{
			I2C_STOP_SIGNAL;
			fault =  I2C_SLAVE_ERROR;
		}

	}
	else
	{
		return I2C_BUS_BUSY;
	}

	while(I2C_CHECK_BUS == BUS_BUSY)
	{
		//STOP
	};

	NVIC_EnableIRQ(i2c_irqs[ch]);

	return I2C_NO_FAULT;

}
