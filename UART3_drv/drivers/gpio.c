/*******************************************************************************
  @file     gpio.c
  @brief    Gpio Driver
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "gpio.h"
#include "MK64F12.h"
#include "core_cm4.h"
#include "hardware.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define PORT2_SIM_SCGC5_MASK(p) (SIM_SCGC5_PORTA_MASK << (((p) >> 5) & 0x07))

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void interruptHandler(uint8_t port);

/*******************************************************************************
 * PRIVATE VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static PORT_Type *ports[] = PORT_BASE_PTRS;
static GPIO_Type *gpioPorts[] = GPIO_BASE_PTRS;
static void (*callbacks[5][32])(void);

/*******************************************************************************
 *                        GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

void gpioMode(pin_t pin, uint8_t mode)
{

	SIM->SCGC5 |= PORT2_SIM_SCGC5_MASK(pin);

	uint8_t port = PIN2PORT(pin);
	uint8_t num = PIN2NUM(pin);

	ports[port]->PCR[num] = 0x0;
	ports[port]->PCR[num] |= PORT_PCR_MUX(1); //Set MUX to GPIO

	if (mode == OUTPUT) // Output
	{
		ports[port]->PCR[num] &= ~PORT_PCR_PE(0);
		gpioPorts[port]->PDDR |= (1 << num);
	}
	else // Input
	{
		if (mode == INPUT)
		{
			ports[port]->PCR[num] &= ~PORT_PCR_PE(0);
		}
		else
		{
			ports[port]->PCR[num] |= PORT_PCR_PE(1);
			if (mode == INPUT_PULLUP)
			{
				ports[port]->PCR[num] |= PORT_PCR_PS(1);
			}
			else
			{
				ports[port]->PCR[num] &= ~PORT_PCR_PS(0);
			}
		}
		gpioPorts[port]->PDDR &= ~(1 << num);
	}
}

void gpioToggle(pin_t pin)
{
	uint8_t port = PIN2PORT(pin);
	uint8_t num = PIN2NUM(pin);
	gpioPorts[port]->PTOR |= (1 << num);
}

bool gpioRead(pin_t pin)
{
	uint8_t port = PIN2PORT(pin);
	uint8_t num = PIN2NUM(pin);
	bool ret = (bool)(gpioPorts[port]->PDIR & (1 << num));
	return ret;
}

void gpioWrite(pin_t pin, bool value)
{
	uint8_t port = PIN2PORT(pin);
	uint8_t num = PIN2NUM(pin);
	gpioPorts[port]->PDOR = (gpioPorts[port]->PDOR & ~(1 << num)) | (value << num);
}

bool gpioIRQ(pin_t pin, uint8_t irqMode, pinIrqFun_t irqFun)
{
	uint8_t port = PIN2PORT(pin);
	uint8_t num = PIN2NUM(pin);

	ports[port]->PCR[num] |= PORT_PCR_IRQC(irqMode);
	NVIC_EnableIRQ(PORTA_IRQn + port);

	callbacks[port][num] = irqFun;
	bool result = NVIC_GetEnableIRQ(PORTA_IRQn + port); // not implemented yet
	return result;
}

bool PORT_ClearInterruptFlag(pin_t pin)
{
	uint8_t port = PIN2PORT(pin);
	uint8_t num = PIN2NUM(pin);
	ports[port]->PCR[num] |= PORT_PCR_ISF_MASK;
	return true;
}

/*******************************************************************************
 *                       LOCAL FUNCTION DEFINITIONS
 ******************************************************************************/

void interruptHandler(uint8_t port)
{
	int i;
	uint32_t isfr = ports[port]->ISFR;
	for (i = 0; i < 32; i++)
	{
		if (callbacks[port][i] && (isfr & 0x1))
		{
			ports[port]->PCR[i] |= PORT_PCR_ISF_MASK;
			callbacks[port][i]();
			break;
		}
		isfr >>= 1;
	}
}

__ISR__ PORTA_IRQHandler(void)
{
	interruptHandler(0);
}

__ISR__ PORTB_IRQHandler(void)
{
	interruptHandler(1);
}

__ISR__ PORTC_IRQHandler(void)
{
	interruptHandler(2);
}

__ISR__ PORTD_IRQHandler(void)
{
	interruptHandler(3);
}

__ISR__ PORTE_IRQHandler(void)
{
	interruptHandler(4);
}
