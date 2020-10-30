/*******************************************************************************
  @file     UART.c
  @brief    UART Driver for K64F. Non-Blocking and using FIFO feature
  @author   Grupo 2
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "uart.h"
#include "MK64F12.h"
#include "hardware.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_DEFAULT_BAUDRATE 9600
#define UART_HAL_DEFAULT_BAUDRATE 9600
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void UART_SetBaudRate(UART_Type * uart, uint32_t baudrate);

/*******************************************************************************
 * PRIVATE VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/





/*******************************************************************************
 *                        GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

void uartInit (uint8_t id, uart_cfg_t config)
{

	UART_Type * ptr_s[] = UART_BASE_PTRS;
	UART_Type * ptr = ptr_s[id];

	PORT_Type * uart_port = PORTC;
	uint8_t rx = 16;
	uint8_t tx = 17;

	/* VER CUAL PONER*/

	//SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
	//SIM->SCGC4 |= SIM_SCGC4_UART1_MASK;
	//SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
	SIM->SCGC4 |= SIM_SCGC4_UART3_MASK;
	//SIM->SCGC1 |= SIM_SCGC1_UART4_MASK;
	//SIM->SCGC1 |= SIM_SCGC1_UART5_MASK;

	//NVIC_EnableIRQ(UART0_RX_TX_IRQn);
	//NVIC_EnableIRQ(UART1_RX_TX_IRQn);
	//NVIC_EnableIRQ(UART2_RX_TX_IRQn);
	NVIC_EnableIRQ(UART3_RX_TX_IRQn);
	//NVIC_EnableIRQ(UART4_RX_TX_IRQn);
	//NVIC_EnableIRQ(UART5_RX_TX_IRQn);


	UART_SetBaudRate(ptr, config.baudrate);

	uart_port->PCR[rx] = 0;
	uart_port->PCR[tx] = 0;
	uart_port->PCR[rx] |= PORT_PCR_MUX(3); // Set alternative uart
	uart_port->PCR[tx] |= PORT_PCR_MUX(3);

	uart_port->PCR[rx] |= PORT_PCR_IRQC(0); // Disable interrupts
	uart_port->PCR[tx] |= PORT_PCR_IRQC(0);

	ptr->C2 = UART_C2_TE_MASK | UART_C2_RE_MASK;
}



uint8_t uartIsRxMsg(uint8_t id)
{

}



uint8_t uartGetRxMsgLength(uint8_t id)
{

}


uint8_t uartReadMsg(uint8_t id, char* msg, uint8_t cant)
{

}


uint8_t uartWriteMsg(uint8_t id, const char* msg, uint8_t cant)
{

}


uint8_t uartIsTxMsgComplete(uint8_t id)
{

}

unsigned char UART_Recieve_Data(void)
{
	while(((UART0->S1)& UART_S1_RDRF_MASK) ==0); // Espero recibir un caracter
	return(UART0->D); //Devuelvo el caracter recibido
}

void UART_Send_Data(unsigned char tx_data)
{
	while(((UART0->S1)& UART_S1_TDRE_MASK) ==0); //Puedo Transmitir ?
	UART0->D = tx_data; // Transmito
}




/*******************************************************************************
 *                       LOCAL FUNCTION DEFINITIONS
 ******************************************************************************/

void UART_SetBaudRate(UART_Type * uart, uint32_t baudrate)
{
	uint16_t sbr, brfa;
	uint32_t clock;

	clock = ((uart == UART0) || (uart == UART1))? (__CORE_CLOCK__):(__CORE_CLOCK__ >> 1);

	baudrate = ((baudrate == 0)?(UART_HAL_DEFAULT_BAUDRATE):
			((baudrate > 0x1FFF)?(UART_DEFAULT_BAUDRATE):(baudrate)));

	sbr = clock / (baudrate<<4);
	brfa = (clock<<1) / baudrate - (sbr<<5);

	uart->BDH = UART_BDH_SBR(sbr >> 8);
	uart->BDL = UART_BDL_SBR(sbr);
	uart->C4 = (uart->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa);
}
