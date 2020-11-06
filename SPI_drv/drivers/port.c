/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////
#include "port.h"
#include "hardware.h"
#include "Assert.h"
#include "CPUTimeMeasurement.h"

/*******************************************************************************
 *                      Local variable definitions ('static') 
 ******************************************************************************/


static PORT_Type * ports[] = PORT_BASE_PTRS;

/*******************************************************************************
 *                        GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

void PORT_GetPinDefaultConfig(PORT_Config * config)
{
	config->ds = PORT_LowDriveStrength;
	config->filter = PORT_PassiveFilterDisable;
	config->interrupt = PORT_InterruptOrDMADisabled;
	config->lk =  PORT_UnlockRegister;
	config->mux = PORT_MuxAlt0;
	config->od = PORT_OpenDrainDisable;
	config->pull = PORT_PullDisable;
	config->sr = PORT_FastSlewRate;
}

void PORT_PinConfig (PORT_Instance n, uint32_t pin, PORT_Config *config, PORT_Mux mux)
{
	ASSERT(n<FSL_FEATURE_SOC_PORT_COUNT);

	//* Clock gating
	uint32_t PORT_SCG[] = {SIM_SCGC5_PORTA_MASK, 
                            SIM_SCGC5_PORTB_MASK, 
                            SIM_SCGC5_PORTC_MASK, 
                            SIM_SCGC5_PORTD_MASK, 
                            SIM_SCGC5_PORTE_MASK};                   
	SIM->SCGC5 |= PORT_SCG[n];

    //* Setting configurations for the pin
	ports[n]->PCR[pin] = 	PORT_PCR_SRE(config->sr) |          //* Slew Rate Enable (0: Fast slew rate or 1: Low slew rate)
							PORT_PCR_PFE(config->filter) |      //* Passive Filter Enable (0: Passive input filter is disabled or 1: enabled)
							PORT_PCR_ODE(config->od) |          //* Open Drain Enable (0: Open drain output is disabled or 1: enabled)
							PORT_PCR_DSE(config->ds) |          //* Drive Strength Enable (0: Low drive strength is configured or 1: High drive strength is configured)  
							PORT_PCR_PS (config->pull) |        //* Pull Select (0: Internal pulldown resistor or 1: Internal pullup resistor)
							PORT_PCR_MUX(config->mux) |         //* Configures the alternative
							PORT_PCR_LK (config->lk) |          //* Lock Register (0: Pin Control Register fields [15:0] are not locked or 1: locked)
						    PORT_PCR_IRQC(config->interrupt);   //* Interrupt Configuration allows you to config the interruption.

	//* Enable or disable internal pull resistor
	if(config->pull == PORT_PullDisable)
		ports[n]->PCR[pin] &= ~PORT_PCR_PE_MASK;
	else
		ports[n]->PCR[pin] |= PORT_PCR_PE_MASK;

    //* Config the alternative on the mux
    ports[n]->PCR[pin] &= ~PORT_PCR_MUX_MASK;
	ports[n]->PCR[pin] |= PORT_PCR_MUX(mux);
}













void PORT_MultiplePinsConfig (PORT_Instance n, uint32_t mask, PORT_Config *config)
{
	ASSERT(n<FSL_FEATURE_SOC_PORT_COUNT);

	uint32_t PCR = 	PORT_PCR_SRE(config->sr) | 
					PORT_PCR_PFE(config->filter) | 
					PORT_PCR_ODE(config->od) | 
					PORT_PCR_DSE(config->ds) | 
					PORT_PCR_PS (config->pull) | 
					PORT_PCR_MUX(config->mux) | 
					PORT_PCR_LK (config->lk) | 
					PORT_PCR_IRQC(config->interrupt);

	ports[n]->GPCLR = PORT_GPCLR_GPWD(PCR) | PORT_GPCLR_GPWE(mask);
	ports[n]->GPCHR = PORT_GPCLR_GPWD(PCR) | PORT_GPCLR_GPWE(mask);

}



void    PORT_PinInterruptConfig (PORT_Instance n, uint32_t pin, PORT_Interrupt interrupt)
{
	ASSERT(n<FSL_FEATURE_SOC_PORT_COUNT);

	ports[n]->PCR[pin] &= ~PORT_PCR_IRQC_MASK;
	ports[n]->PCR[pin] |= PORT_PCR_IRQC(interrupt);
}

uint32_t PORT_GetPinInterruptFlag (PORT_Instance n,uint32_t pin)
{
	ASSERT(n<FSL_FEATURE_SOC_PORT_COUNT);
	return (ports[n]->ISFR>>pin)&1;
}

uint32_t PORT_GetPinsInterruptFlags (PORT_Instance n)
{
	ASSERT(n<FSL_FEATURE_SOC_PORT_COUNT);
	return ports[n]->ISFR;
}

void 	PORT_ClearPinInterruptFlag (PORT_Instance n, uint32_t pin)
{
	ASSERT(n<FSL_FEATURE_SOC_PORT_COUNT);
	//W1C????

}