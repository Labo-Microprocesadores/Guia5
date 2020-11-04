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


/************************************************
*            !SOLO PARA GUIA ESPIRITUAL
*************************************************/
// setPinConfigurations(SPI_n);

// 	MSTRCFG_t master_config;
// 	MSTRCFG(&master_config);

// 	SPI_Type * base = SPIPtrs[SPI_n];

// 	HALTStopTransfers(SPI_n);

// 	base->MCR = 0;
// 	base->MCR &=~ SPI_MCR_MDIS_MASK;
// 	base->MCR |= SPI_MCR_MDIS(master_config.MCR_isModuleDisabled);
// 	uint32_t temp = 0;
// 	temp = base->MCR & (~(  SPI_MCR_MSTR_MASK |
// 							SPI_MCR_CONT_SCKE_MASK |
// 							SPI_MCR_MTFE_MASK |
// 							SPI_MCR_ROOE_MASK |
// 							SPI_MCR_SMPL_PT_MASK |
// 	                        SPI_MCR_DIS_TXF_MASK |
// 							SPI_MCR_DIS_RXF_MASK));

// 	base->MCR = temp |
// 					SPI_MCR_MSTR(master_config.MCR_isMaster) |
// 					SPI_MCR_CONT_SCKE(master_config.MCR_ContinousSerialClockEnable) |
// 	                SPI_MCR_MTFE(master_config.MCR_ModifiedTransferFormatEnable) |
// 	                SPI_MCR_ROOE(master_config.MCR_ReceiveFIFOverflowOverwriteEnable) |
// 					SPI_MCR_SMPL_PT(master_config.MCR_Sample_Point) |
// 	                SPI_MCR_DIS_TXF(master_config.MCR_isTxfifoDisabled) |
// 					SPI_MCR_DIS_RXF(master_config.MCR_isRxfifoDisabled);

// 	base->CTAR[master_config.whichCTAR]= 0;
// 	temp = base->CTAR[master_config.whichCTAR] & ~(  SPI_CTAR_BR_MASK |
// 													 SPI_CTAR_PBR_MASK |
// 													 SPI_CTAR_FMSZ_MASK |
// 									 	 	 	 	 SPI_CTAR_CPOL_MASK |
// 													 SPI_CTAR_CPHA_MASK |
// 													 SPI_CTAR_LSBFE_MASK|
// 													 SPI_CTAR_ASC_MASK|
// 													 SPI_CTAR_PASC_MASK|
// 													 SPI_CTAR_CSSCK_MASK|
// 													 SPI_CTAR_PCSSCK_MASK|
// 													 SPI_CTAR_DT_MASK|
// 													 SPI_CTAR_PDT_MASK);

// 	base->CTAR[master_config.whichCTAR] = temp | SPI_CTAR_BR(master_config.CTAR_BR) |
// 												 SPI_CTAR_PBR(master_config.CTAR_BRPRESC) |
// 												 SPI_CTAR_FMSZ(master_config.CTAR_FrameSize - 1) |
// 								   	   	   	   	 SPI_CTAR_CPOL(master_config.CTAR_CPOL) |
// 												 SPI_CTAR_CPHA(master_config.CTAR_CPHA) |
// 												 SPI_CTAR_LSBFE(master_config.CTAR_LSBFE)|
// 												 SPI_CTAR_ASC(master_config.CTAR_DelayAfterSCKtoNegPCSScaler)|
// 												 SPI_CTAR_PASC(master_config.CTAR_DelayAfterSCKtoNegPCSPrescaler)|
// 												 SPI_CTAR_CSSCK(master_config.CTAR_DelayPCStoSCKScaler)|
// 												 SPI_CTAR_PCSSCK(master_config.CTAR_DelayPCStoSCKPrescaler)|
// 												 SPI_CTAR_DT(master_config.CTAR_DelayAfterTransferScaler)|
// 												 SPI_CTAR_PDT(master_config.CTAR_DelayAfterTransferPrescaler);

// 	setPCSActiveLow(SPI_n);

// 	HALTStartTransfers(SPI_n);




static void SPI_slave_init(uint8_t id, spi_config_t config)
{
    
}
