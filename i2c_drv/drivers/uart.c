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

#define MAX_BUFFER_LEN 101

#define ISR_TDRE(x) (((x) & UART_S1_TDRE_MASK) != 0x0)
#define ISR_RDRF(x) (((x) & UART_S1_RDRF_MASK) != 0x0)
#define ISR_TC(x) (((x) & UART_S1_TC_MASK) != 0x0)
#define ISR_IDLE(x) (((x) & UART_S1_IDLE_MASK) != 0x0)
#define ISR_OR(x) (((x) & UART_S1_OR_MASK) != 0x0)
#define ISR_NF(x) (((x) & UART_S1_NF_MASK) != 0x0)
#define ISR_FE(x) (((x) & UART_S1_FE_MASK) != 0x0)
#define ISR_PF(x) (((x) & UART_S1_PF_MASK) != 0x0)

#define MSG_LEN(x,y,z) (((x)+(z)-(y)) % ((z) - 1 )) // MSG_LEN(rear, front, max_len)

#define UART_PORTS	{PORTB, PORTC, PORTD, PORTC, PORTE}
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void UART_set_baud_rate(UART_Type * uart, uint32_t baudrate);
void UART_set_parity(UART_Type * uart, uart_parity_t parity);
void UART_rx_tx_irq_handler(UART_Type * p_uart, uint8_t id);

/*******************************************************************************
 * PRIVATE VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static uint8_t buffer_out[UART_CANT_IDS][MAX_BUFFER_LEN];
static uint8_t p_out_rear[UART_CANT_IDS], p_out_front[UART_CANT_IDS];

static uint8_t buffer_in[UART_CANT_IDS][MAX_BUFFER_LEN];
static uint8_t p_in_rear[UART_CANT_IDS], p_in_front[UART_CANT_IDS];

static bool uart_use[UART_CANT_IDS] = {false};
/*******************************************************************************
 *                        GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

void UART_init (uint8_t id, uart_cfg_t config)
{
	uint8_t i;

	/********* Tomo el puerto ***********/
	PORT_Type * arr_uart_ports[] = UART_PORTS;
	uint8_t ports_number[] = {16,3,2,16,25};
	PORT_Type * uart_port = arr_uart_ports[id];
	uint8_t rx = ports_number[id];
	uint8_t tx = id == 4? ports_number[id]-1:ports_number[id]+1;
	/********************************************/

	UART_Type * ptr_s[] = UART_BASE_PTRS;
	UART_Type * p_uart = ptr_s[id];


	p_out_rear[id] = 0;
	p_out_front[id] = 1;
	p_in_rear[id] = 0;
	p_in_front[id] = 1;
	for(i = 0; i < MAX_BUFFER_LEN; i++)
	{
		buffer_out[id][i] = 0;
		buffer_in[id][i] = 0;
	}
	uart_use[id] = true;

	if(id == 4 || id == 5)
	{
		SIM->SCGC1 |= SIM_SCGC1_UART4_MASK << (id%4);
		NVIC_EnableIRQ(UART4_RX_TX_IRQn+(id%4)*2);
	}
	else
	{
		SIM->SCGC4 |= SIM_SCGC4_UART0_MASK << (id%4);
		NVIC_EnableIRQ(UART0_RX_TX_IRQn+id*2);
	}

	UART_set_baud_rate(p_uart, config.baudrate);
	UART_set_parity(p_uart, config.parity);

	uart_port->PCR[rx] = 0;
	uart_port->PCR[tx] = 0;

	uart_port->PCR[rx] |= PORT_PCR_MUX(3); // Set alternative uart
	uart_port->PCR[tx] |= PORT_PCR_MUX(3);

	uart_port->PCR[rx] |= PORT_PCR_IRQC(0); // Disable interrupts
	uart_port->PCR[tx] |= PORT_PCR_IRQC(0);

	p_uart->C2 = UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK ;

}


bool UART_is_rx_msg(uint8_t id)
{
	return MSG_LEN(p_in_rear[id], p_in_front[id], MAX_BUFFER_LEN) != 0;
}



uint8_t UART_get_rx_msg_length(uint8_t id)
{
	return MSG_LEN(p_in_rear[id], p_in_front[id], MAX_BUFFER_LEN);
}


uint8_t UART_read_msg(uint8_t id, char* msg, uint8_t cant)
{
	uint8_t i,len_read = MSG_LEN(p_in_rear[id], p_in_front[id], MAX_BUFFER_LEN);
	if(len_read > cant)
		len_read = cant;

	for(i = 0; i<len_read; i++)
	{
		msg[i] = buffer_in[id][p_in_front[id]];
		p_in_front[id] = (p_in_front[id] + 1) % (MAX_BUFFER_LEN - 1 );
	}
	return len_read;
}


uint8_t UART_write_msg(uint8_t id, const char* msg, uint8_t cant)
{
	uint8_t len_write = 0;
	UART_Type * ptr_s[] = UART_BASE_PTRS;

	while((cant > len_write) && (((p_out_rear[id] + 2) % (MAX_BUFFER_LEN - 1)) != p_out_front[id])) // Buffer full
	{
		p_out_rear[id] = (p_out_rear[id] + 1) % (MAX_BUFFER_LEN - 1); // Incremento circular
		buffer_out[id][p_out_rear[id]] = msg[len_write];
		len_write++;
	}
	ptr_s[id]->C2 |= UART_C2_TIE_MASK; // Enable tie interrupts
	return len_write;
}


bool UART_is_tx_msg_complete(uint8_t id)
{
	return MSG_LEN(p_out_rear[id], p_out_front[id], MAX_BUFFER_LEN) == 0;
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

void UART_set_baud_rate(UART_Type * uart, uint32_t baudrate)
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

void UART_set_parity(UART_Type * uart, uart_parity_t parity)
{
	uart->C1 = (uart->C1 & ~ UART_C1_PE_MASK) | UART_C1_PE(parity != 0);
	if(parity != 0)
	{
		uart->C1 = (uart->C1 & ~UART_C1_PT_MASK) | UART_C1_PT(parity & 0x1);
	}
}

void UART_rx_tx_irq_handler(UART_Type * p_uart, uint8_t id)
{
	unsigned char tmp, i, rx_data, tx_data;
	i = id;
	tmp=p_uart->S1;
	if(ISR_TDRE(tmp))
	{
		uint8_t msg_len = MSG_LEN(p_out_rear[i], p_out_front[i], MAX_BUFFER_LEN);
		if(msg_len != 0) // Si tengo caracteres en la cola lo mando
		{
			tx_data = buffer_out[i][p_out_front[i]];
			p_out_front[i] = (p_out_front[i] + 1) % (MAX_BUFFER_LEN - 1 );
			p_uart->D = tx_data; // Transmito

			if(msg_len == 1) //Clear tie interrupt when buffer is empty
				p_uart->C2 = (p_uart->C2 & ~UART_C2_TIE_MASK);
		}
		else
		{
			//ERROR no message to send,
		}

	}
	if(ISR_TC(tmp)) //creo que no vale la pena usarlo
	{

	}
	if(ISR_RDRF(tmp))
	{
		rx_data=p_uart->D;
		if (((p_in_rear[i] + 2) % (MAX_BUFFER_LEN - 1)) != p_in_front[i]) // Buffer full
		{
			p_in_rear[i] = (p_in_rear[i] + 1) % (MAX_BUFFER_LEN - 1); // Incremento circular
			buffer_in[i][p_in_rear[i]] = rx_data;
		}
	}
	if(ISR_IDLE(tmp)) //creo que no vale la pena usarlo
	{

	}
	if(ISR_OR(tmp))
	{

	}
	if(ISR_NF(tmp))
	{

	}
	if(ISR_FE(tmp))
	{

	}
	if(ISR_PF(tmp))
	{

	}
}

__ISR__ UART0_RX_TX_IRQHandler (void)
{
	UART_rx_tx_irq_handler(UART0, 0);
}

__ISR__ UART1_RX_TX_IRQHandler (void)
{
	UART_rx_tx_irq_handler(UART1, 1);
}

__ISR__ UART2_RX_TX_IRQHandler (void)
{
	UART_rx_tx_irq_handler(UART2, 2);
}

__ISR__ UART3_RX_TX_IRQHandler (void)
{
	UART_rx_tx_irq_handler(UART3, 3);
}

__ISR__ UART4_RX_TX_IRQHandler (void)
{
	UART_rx_tx_irq_handler(UART4, 4);
}
