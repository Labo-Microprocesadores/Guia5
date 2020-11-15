/***************************************************************************//**
  @file     UART.h
  @brief    UART Driver for K64F. Non-Blocking and using FIFO feature
  @author   Nicolás Magliola & Grupo 2
 ******************************************************************************/

#ifndef _UART_H_
#define _UART_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_CANT_IDS   4


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum uart_parity_t {
	UART_PARITY_NONE,
	UART_PARITY_EVEN,
	UART_PARITY_ODD,
	//UART_PARITY_MARK,
	//UART_PARITY_SPACE
} uart_parity_t;

typedef enum uart_data_bits_t {
	UART_DATA_BITS_5,
	UART_DATA_BITS_6,
	UART_DATA_BITS_7,
	UART_DATA_BITS_8,
	UART_DATA_BITS_9
} uart_data_bits_t;

typedef enum uart_stop_bits_t {
	UART_STOP_BITS_1,
	UART_STOP_BITS_2
}uart_stop_bits_t;

typedef enum uart_error_t{
	UART_FULLBUFFER_ERROR
}uart_error_t;

typedef struct {
    uint32_t baudrate;
    uart_parity_t parity;
    uart_data_bits_t databits;
    uart_stop_bits_t stop;
} uart_cfg_t;


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

unsigned char UART_Recieve_Data(void);
void UART_Send_Data(unsigned char tx_data);

/**
 * @brief Initialize UART driver
 * @param id UART's number
 * @param config UART's configuration (baudrate, parity, etc.)
*/
void UART_init (uint8_t id, uart_cfg_t config);

/**
 * @brief Check if a new byte was received
 * @param id UART's number
 * @return A new byte has been received
*/
bool UART_is_rx_msg(uint8_t id);

/**
 * @brief Check how many bytes were received
 * @param id UART's number
 * @return Quantity of received bytes
*/
uint8_t UART_get_rx_msg_length(uint8_t id);

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
*/
uint8_t UART_read_msg(uint8_t id, char* msg, uint8_t cant);

/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transfered
 * @param cant Desired quantity of bytes to be transfered
 * @return Real quantity of bytes to be transfered
 */
uint8_t UART_write_msg(uint8_t id, const char* msg, uint8_t cant);

/**
 * @brief Check if all bytes were transfered
 * @param id UART's number
 * @return All bytes were transfered
*/
bool UART_is_tx_msg_complete(uint8_t id);


/*******************************************************************************
 ******************************************************************************/

#endif // _UART_H_
