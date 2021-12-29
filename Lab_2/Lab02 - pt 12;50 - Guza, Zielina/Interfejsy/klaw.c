#include "klaw.h"
#include "frdm_bsp.h"

extern uint8_t key_down;
extern uint8_t buf_key;

void Klaw_Init(void)
{
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;		//Włączenie portu A
	PORTA->PCR[9] |= PORT_PCR_MUX(1);			// PTA5 - PTA12: GPIO
	PORTA->PCR[9] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[10] |= PORT_PCR_MUX(1);
	PORTA->PCR[10] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[11] |= PORT_PCR_MUX(1);
	PORTA->PCR[11] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[12] |= PORT_PCR_MUX(1);
	PORTA->PCR[12] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[5] |= PORT_PCR_MUX(1);
	PORTA->PCR[5] &= (~PORT_PCR_SRE_MASK);
	PORTA->PCR[5] &= (~PORT_PCR_DSE_MASK);
	PORTA->PCR[6] |= PORT_PCR_MUX(1);
	PORTA->PCR[6] &= (~PORT_PCR_SRE_MASK);
	PORTA->PCR[6] &= (~PORT_PCR_DSE_MASK);
	PORTA->PCR[7] |= PORT_PCR_MUX(1);
	PORTA->PCR[7] &= (~PORT_PCR_SRE_MASK);
	PORTA->PCR[7] &= (~PORT_PCR_DSE_MASK);
	PORTA->PCR[8] |= PORT_PCR_MUX(1);
	PORTA->PCR[8] &= (~PORT_PCR_SRE_MASK);
	PORTA->PCR[8] &= (~PORT_PCR_DSE_MASK);
	PTA->PDOR |= PTA9_12_MASK;
	PTA->PDDR |= PTA5_8_MASK;		//PTA5 - PTA8: wyjścia
	PTA->PDOR |= PTA5_8_MASK;
}

void Klaw_S2_Int(void)
{
	PTA->PCOR = 1<<5;			// R4=0
	PORTA -> PCR[10] |= PORT_PCR_IRQC(0xa);
	NVIC_ClearPendingIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTA_IRQn);

	
}
