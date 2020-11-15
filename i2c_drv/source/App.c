/***************************************************************************/ /**
  @file     App.c
  @brief    Application functions
  @author   Grupo 2
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

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

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void send_msg(void);
int idtimer = 0;
/*******************************************************************************
 *******************************************************************************
                        FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init(void)
{

	uart_cfg_t config = {9600, UART_PARITY_NONE, UART_DATA_BITS_8, UART_STOP_BITS_1};
	UART_init(0, config);
	Led_Init();
	UART_init(3, config);
	Timer_Init();
	idtimer = Timer_AddCallback(&send_msg, 5000, false);
	send_msg();

	i2cInit(I2C_0);

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
	I2C_COM_CONTROL testing;
	testing.callback=toggle_led;
	uint8_t data [3]={1,2,3};
	testing.data = data;
	testing.data_size = 3;
	testing.register_address= 0x2A;//0x0E;
	testing.slave_address = 0x1D;
	i2cWriteMsg(&testing);
	UART_write_msg(0,'hola\r\n',6);
}


void toggle_led(void)
{
	Led_Toggle(LED_RED);
}



/*******************************************************************************
 *******************************************************************************/
void send_msg(void)
{
	static const char espmsg[1] = {0x04};
	UART_write_msg(3, (const char *)&espmsg, 1);
	//UART_write_msg(0, "Mande mensaje\r\n", 15);
	Timer_Pause(idtimer);
}
