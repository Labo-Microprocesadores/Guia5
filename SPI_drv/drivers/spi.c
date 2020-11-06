/*******************************************************************************
  @file     spi.c
  @brief    spi driver
  @author   Grupo 2 - Lab de Micros
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "spi.h"
#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void SPI_master_init(uint8_t id, spi_config_t config);
static void SPI_slave_init(uint8_t id, spi_config_t config);
static void SPI0_IRQHandler(void);

/*******************************************************************************
 * PRIVATE VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *                        GLOBAL FUNCTION DEFINITIONS
 ******************************************************************************/

void SPI_init(uint8_t id, spi_config_t config, enum deviceRole role)
{
    NVIC_EnableIRQ(PORTD_IRQn);
    switch(id)
    {
		case SPI0:
			sim_ptr->SCGC6 |= SIM_SCGC6_SPI0(ENABLE); // activo clock gating
			break;
		case SPI1:
			sim_ptr->SCGC6 |= SIM_SCGC6_SPI1(ENABLE);
			break;
		case SPI2:
			sim_ptr->SCGC3 |= SIM_SCGC3_SPI2(ENABLE);
			break;
	}
    setSPIConfig(id, SPI_CONFIG);
    if(role == MASTER)
    {
        SPI_master_init(id, config);
    }
    else if(role == SLAVE)
    {
        SPI_slave_init(id, config);
    }
    else
    {

    }
}

/*******************************************************************************
 *                       LOCAL FUNCTION DEFINITIONS
 ******************************************************************************/

static void SPI_master_init(uint8_t id, spi_config_t config)
{
    uint32_t pinNum = PIN2NUM(id);
    port->PCR[num] = 0x00;
    PORT_Type *port = portPtrs[PIN2PORT(PIN_PCS0)];
	SIM->SCGC5 |= sim_port[PIN2PORT(PIN_PCS0)];
    port->PCR[pinNum] &= ~PORT_PCR_MUX_MASK;
	port->PCR[pinNum] &= ~PORT_PCR_IRQC_MASK;
    port->PCR[pinNum] |= PORT_PCR_MUX(SPI_ALTERNATIVE); // ENABLE SPI
	port->PCR[pinNum] |= PORT_PCR_IRQC(GPIO_IRQ_MODE_DISABLE);
	port->PCR[pinNum] &= ~(HIGH<<1); //PULL ENABLE en 0
}

static void SPI_slave_init(uint8_t id, spi_config_t config)
{
    
}


static void SPI0_IRQHandler(void)
{
    //ME GUSTA LA FORMA DEL DRIVER OFICIAL, MUY BONITA Y MANEJA TX Y RX DE UNA! (PROBABLEMENTE HAYA PROBLAMAS CON LOS FLAGS QUE NO VAMOS A TENER)
}
