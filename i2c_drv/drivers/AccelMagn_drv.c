


#include "AccelMagn_drv.h"
#include "i2c.h"
#include "Led.h"

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR 	0x1D//0x1E // with pins SA0=0, SA1=0

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS 		0x00
#define FXOS8700CQ_WHOAMI 		0x0D
#define FXOS8700CQ_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_CTRL_REG1 	0x2A
#define FXOS8700CQ_M_CTRL_REG1 	0x5B
#define FXOS8700CQ_M_CTRL_REG2 	0x5C
#define FXOS8700CQ_WHOAMI_VAL 	0xC7

// number of bytes to be read from the FXOS8700CQ
#define FXOS8700CQ_READ_LEN 13 // status plus 6 channels = 13 bytes


I2C_COM_CONTROL i2c_com;
static read_data * r_data;
static bool finish = false;
uint8_t Buffer[FXOS8700CQ_READ_LEN]; // read buffer


/*******************************************************************************
                        LOCAL SCOPE FUNCTION PROTOTYPES
 *******************************************************************************/
void AccelMagn_readSensors(SRAWDATA *pAccelData, SRAWDATA *pMagnData);
void callback_init (void);

/*******************************************************************************
                        GLOBAL SCOPE FUNCTION DEFINITIONS
 *******************************************************************************/


//This function configures the FXOS8700CQ for 200-Hz hybrid mode meaning that
// both accelerometer and magnetometer data are provided at the 200-Hz rate.
// function configures FXOS8700CQ combination accelerometer and magnetometer sensor
//I2C_STATUS AccelMagn_init(void)
I2C_STATUS _mqx_ints_FXOS8700CQ_start(void)
{
	//Led_Toggle(LED_RED);
	i2cInit(I2C_0);
	uint8_t databyte;

	// read and check the FXOS8700CQ WHOAMI register
	if (i2cReadMsgBlocking(&databyte, 1, FXOS8700CQ_WHOAMI, FXOS8700CQ_SLAVE_ADDR)!= I2C_NO_FAULT)
	{
		Led_Toggle(LED_RED);
		return (I2C_ERROR);
	}
	if (databyte != FXOS8700CQ_WHOAMI_VAL)
	{
		//Led_Toggle(LED_RED);
		return (I2C_ERROR);
	}
	//Led_Toggle(LED_RED);

	// write 0000 0000 = 0x00 to accelerometer control register 1 to place FXOS8700CQ into standby
	// [7-1] = 0000 000
	// [0]: active=0
	databyte = 0x00;
	finish = false;
	i2c_com.callback = callback_init;
	i2c_com.data = &databyte;
	i2c_com.data_size = 1;
	i2c_com.slave_address =FXOS8700CQ_SLAVE_ADDR;
	i2c_com.register_address = FXOS8700CQ_CTRL_REG1;

	i2cWriteMsg(&i2c_com);

	int test = 1;
	while (finish == false)
	{
		if (test){
			Led_Toggle(LED_RED);
			test = 0;
		}
		if(i2c_com.fault != I2C_NO_FAULT)
		{
			//Led_Toggle(LED_RED);
			return (I2C_ERROR);
		}
	}


	// write 0001 1111 = 0x1F to magnetometer control register 1
	// [7]: m_acal=0: auto calibration disabled
	// [6]: m_rst=0: no one-shot magnetic reset
	// [5]: m_ost=0: no one-shot magnetic measurement
	// [4-2]: m_os=111=7: 8x oversampling (for 200Hz) to reduce magnetometer noise
	// [1-0]: m_hms=11=3: select hybrid mode with accel and magnetometer active
	databyte = 0x9F;//0x1F;
	finish = false;
	i2c_com.register_address = FXOS8700CQ_M_CTRL_REG1;
	i2cWriteMsg(&i2c_com);
	while (finish == false)
	{
		if(i2c_com.fault != I2C_NO_FAULT)
		{
			//Led_Toggle(LED_RED);
			return (I2C_ERROR);
		}
	}


	// write 0010 0000 = 0x20 to magnetometer control register 2
	// [7]: reserved
	// [6]: reserved
	// [5]: hyb_autoinc_mode=1 to map the magnetometer registers to follow the accelerometer registers
	// [4]: m_maxmin_dis=0 to retain default min/max latching even though not used
	// [3]: m_maxmin_dis_ths=0
	// [2]: m_maxmin_rst=0
	// [1-0]: m_rst_cnt=00 to enable magnetic reset each cycle
	databyte = 0x20;
	finish = false;
	i2c_com.register_address = FXOS8700CQ_M_CTRL_REG2;
	i2cWriteMsg(&i2c_com);
	while (finish == false)
	{
		if(i2c_com.fault != I2C_NO_FAULT)
		{
			//Led_Toggle(LED_RED);
			return (I2C_ERROR);
		}
	}


	// write 0000 0001= 0x01 to XYZ_DATA_CFG register
	// [7]: reserved
	// [6]: reserved
	// [5]: reserved
	// [4]: hpf_out=0
	// [3]: reserved
	// [2]: reserved
	// [1-0]: fs=01 for accelerometer range of +/-4g range with 0.488mg/LSB
	databyte = 0x01;
	finish = false;
	i2c_com.register_address = FXOS8700CQ_XYZ_DATA_CFG;
	i2cWriteMsg(&i2c_com);
	while (finish == false)
	{
		if(i2c_com.fault != I2C_NO_FAULT)
		{
			//Led_Toggle(LED_RED);
			return (I2C_ERROR);
		}
	}


	// write 0000 1101 = 0x0D to accelerometer control register 1
	// [7-6]: aslp_rate=00
	// [5-3]: dr=001 for 200Hz data rate (when in hybrid mode)
	// [2]: lnoise=1 for low noise mode
	// [1]: f_read=0 for normal 16 bit reads
	// [0]: active=1 to take the part out of standby and enable sampling
	databyte = 0x0D;
	finish = false;

	i2c_com.register_address = FXOS8700CQ_CTRL_REG1;

	i2cWriteMsg(&i2c_com);

	while (finish == false)
	{
		if(i2c_com.fault != I2C_NO_FAULT)
		{
			//Led_Toggle(LED_RED);
			return (I2C_ERROR);
		}
	}
	// normal return
	return (I2C_OK);
}



void AccelMagn_getData(read_data * data)
{
	// read FXOS8700CQ_READ_LEN=13 bytes (status byte and the six channels of data)

	r_data = data;
	i2c_com.callback = AccelMagn_readSensors;
	i2c_com.data = Buffer;
	i2c_com.data_size = FXOS8700CQ_READ_LEN;
	i2c_com.register_address = FXOS8700CQ_STATUS;
	i2c_com.slave_address =FXOS8700CQ_SLAVE_ADDR;

	i2cReadMsg(&i2c_com);

	// normal return
	return;
}

/*******************************************************************************
                        LOCAL SCOPE FUNCTION DEFINITIONS
 *******************************************************************************/

/*
This function performs a block read of the status, accelerometer and magnetometer
measurement registers and places the bytes read into the structures of type SRAWDATA
as signed short integers.
Note that this function assumes that the hyb_autoinc_mode bit has been set to enable the
reading of all accelerometer and magnetometer data in a single-burst, read operation.
*/

// read status and the three channels of accelerometer and magnetometer data from
// FXOS8700CQ (13 bytes)
void AccelMagn_readSensors(SRAWDATA *pAccelData, SRAWDATA *pMagnData)
{
	// read FXOS8700CQ_READ_LEN=13 bytes (status byte and the six channels of data)
	if(i2c_com.fault == I2C_NO_FAULT)
	{
		// copy the 14 bit accelerometer byte data into 16 bit words
		r_data->pAccelData->x = (int16_t)(((Buffer[1] << 8) | Buffer[2]))>> 2;
		r_data->pAccelData->y = (int16_t)(((Buffer[3] << 8) | Buffer[4]))>> 2;
		r_data->pAccelData->z = (int16_t)(((Buffer[5] << 8) | Buffer[6]))>> 2;
		// copy the magnetometer byte data into 16 bit words
		r_data->pMagnData->x = (Buffer[7] << 8) | Buffer[8];
		r_data->pMagnData->y = (Buffer[9] << 8) | Buffer[10];
		r_data->pMagnData->z = (Buffer[11] << 8) | Buffer[12];

		r_data->error = I2C_OK;
		r_data->callback();
	}
	else
	{
		r_data->error = I2C_ERROR;
	}
}

void callback_init (void)
{
	finish = true;
}




