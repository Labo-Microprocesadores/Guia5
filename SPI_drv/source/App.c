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
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void send_msg(void);

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
	Timer_Init();
	Timer_AddCallback(&send_msg, 1000, false);
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run(void)
{
	static uint8_t msg[105] = {0};
	if(UART_is_rx_msg(0))
	{
		uint8_t mlen =  UART_read_msg(0,&msg, 100);
		for(uint8_t i = 0; i < mlen; i++)
			msg[i]++;
		msg[mlen] = '\r';
		msg[mlen+1] = '\n';
		mlen = UART_write_msg(0, &msg, mlen+2);
	}
}



/*******************************************************************************
 *******************************************************************************/
void send_msg(void)
{
	static char mensaje[] = "Hola mundo\r\n";
	UART_write_msg(0, &mensaje, sizeof(mensaje)/sizeof(mensaje[0]));
}
