/***************************************************************************/ /**
  @file     App.c
  @brief    Application functions
  @author   Grupo 2
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "Posicionamiento.h"
#include "board.h"
#include "uart.h"
#include "Timer.h"
#include "Led.h"
#include "i2c.h"
#include <stdbool.h>
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void toggle_led(void);
void test(void);

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void periodicRefresh(void);
void send_msg(void);
void send_i2c_msg(void);
void callback_init(void);
void read(void);
int idtimer1 = 0;
int idtimer2 = 0;
int idtimer = 0;
//I2C_COM_CONTROL i2c_com;

enum{ROLL_REFRESH, PITCH_REFRESH, ORIENT_REFRESH};
uint8_t fsm = ROLL_REFRESH;

//tim_id_t timPeriodico;
//tim_id_t timerUpdatePos;

roll_t roll_app;
pitching_t pitching_app;
orientation_t orientation_app;

/*******************************************************************************
 *******************************************************************************
                        FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static bool finish = false;
//uint8_t Buffer[13];

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{

	uart_cfg_t config = {9600, UART_PARITY_NONE, UART_DATA_BITS_8, UART_STOP_BITS_1};
	UART_init(0, config);

	Position_InitDrv(test);
	Led_Init();
	UART_init(3, config);
	Timer_Init();
	//i2cInit(I2C_0);
	//idtimer = Timer_AddCallback(&send_msg, 5000, false);
	//send_msg();
	//id2timer = Timer_AddCallback(&send_i2c_msg, 5000, false);

	//idtimer = Timer_AddCallback(&read, 5000, false);
	idtimer1 = Timer_AddCallback(&Position_Update,200,false);
	idtimer2 = Timer_AddCallback(&periodicRefresh,1000,false);

	//i2cInit(I2C_0);

	/*
	uint8_t databyte = 0x00;
	finish = false;
	i2c_com.callback = callback_init;
	i2c_com.data = &databyte;
	i2c_com.data_size = 1;
	i2c_com.slave_address =0x1D;
	i2c_com.register_address = 0x2A;
	*/

	//i2cWriteMsg(&i2c_com);

	/*
	while (finish == false)
	{
		if(i2c_com.fault != I2C_NO_FAULT)
		{
			//return (I2C_ERROR);
		}
	}
	*/
}

void callback_init(void)
{
	finish = true;
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
	static char msg[105] = {0};

	/*
	if(UART_is_rx_msg(3))
	{
		//const char men[] = "Recibi: ";
		uint8_t mlen =  UART_read_msg(3,(char *)&msg, 100);
		//for(uint8_t i = 0; i < mlen; i++){msg[i]+='0';}
		//msg[mlen] = '\r';
		//msg[mlen+1] = '\n';

		if(msg[mlen-1] == 0x03)
		{
			Led_Toggle(LED_RED);
			Timer_Resume(idtimer);
			Timer_Reset(idtimer);
		}
		//UART_write_msg(0, &men, sizeof(men)/sizeof(men[0]));
		//mlen = UART_write_msg(0, &msg, mlen+2);
	}
	*/


	/*
	I2C_COM_CONTROL testing;
	testing.callback=toggle_led;
	uint8_t data [3]={1,2,3};
	testing.data = data;
	testing.data_size = 3;
	testing.register_address= 0x00;//0x2A;//0x0E;
	testing.slave_address = 0x1D;
	i2cWriteMsg(&testing);
	UART_write_msg(0,'hola\r\n',6);
	*/

}


void toggle_led(void)
{
	Led_Toggle(LED_RED);
}



/*******************************************************************************
 *******************************************************************************/

void test(void)
{

}

void periodicRefresh(void)
{
	// Cada 1 segundo refresca uno de los parámetros
	switch (fsm) {
		case ROLL_REFRESH:
			roll_app = Position_GetRoll();
			// ENVIAR A PC NUEVO DATO POR UART
			break;
		case PITCH_REFRESH:
			pitching_app = Position_GetPitch();
			// Enviar a PC NUEVO DATO POR UART
			break;
		case ORIENT_REFRESH:
			orientation_app = Position_GetOrientation();
			// ENVIAR A PC NUEVO DATO POR UART
			break;
	}


	if(fsm == ORIENT_REFRESH)
	{
		fsm = ROLL_REFRESH;
	}
	else
	{
		fsm++;
	}
}

void read(void)
{
	//r_data = data;
	/*
	i2c_com.callback = toggle_led;
	i2c_com.data = Buffer;
	i2c_com.data_size = 13;
	i2c_com.register_address = 0x00;
	i2c_com.slave_address = 0x1D;

	i2cReadMsg(&i2c_com);
	*/

	// normal return
	return;
}

void send_i2c_msg(void)
{
	/*
	I2C_COM_CONTROL testing;
	finish = false;
		testing.callback=toggle_led;
		//uint8_t data [3]={1,2,3};
		uint8_t data = 0x02;
		testing.data = data;
		testing.data_size = 1;
		//testing.data_size = 3;
		testing.register_address= 0x00;//0x2A;//0x0E;
		testing.slave_address = 0x1D;
		i2cWriteMsg(&testing);
		//UART_write_msg(0,'hola\r\n',6);
		while (finish == false)
		{
			if(i2c_com.fault != I2C_NO_FAULT)
			{
				//return (I2C_ERROR);
			}
		}
		*/
}

void send_msg(void)
{
	static const char espmsg[1] = {0x04};
	UART_write_msg(3, (const char *)&espmsg, 1);
	//UART_write_msg(0, "Mande mensaje\r\n", 15);
	Timer_Pause(idtimer);
}
