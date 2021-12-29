/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 4 - TPM/PWM - Kotek i Fuga
					autor: Mariusz Sokolowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
	
#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include "klaw.h"
#include "TPM.h"
#include "tsi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MOD_MAX	40082
#define ZEGAR 1310720
#define DIV_CORE	16

volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;
float	ampl_v;
float freq;
uint16_t	mod_curr=MOD_MAX;
uint16_t	ampl;
uint16_t Tony[]={40082, 37826, 35701, 33702, 31813, 30027, 28339, 26748, 25249, 23830, 22493, 21229, 20041};
uint16_t Oktawa[]={1, 2, 4, 8, 16, 32, 64, 128};
uint8_t	Kotek[]={7,4,4,5,2,2,0,4,7,7,4,4,5,2,2,0,4,0};
uint8_t	Kotek_W[]={4,4,4,4,4,4,2,2,8,4,4,4,4,4,4,2,2,8};
uint8_t Kotek_Gap[]={2,2,2,2,2,2,1,1,4,2,2,2,2,2,2,1,1,10};
uint8_t	Fuga[]={9,7,9,7,5,4,2,1,2,9,7,9,4,5,1,2};
uint8_t	Fuga_W[]={2,2,16,2,2,2,2,4,16,2,2,16,4,4,4,16};
uint8_t Fuga_Gap[]={1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,10};
uint8_t	ton=0;
int8_t	gama=2;
uint8_t sekunda=0;			
uint8_t sekunda_OK=0;	 
uint8_t	gap_ton=0;
uint8_t on_off=255;
uint8_t czas=1;

void SysTick_Handler(void)	// Podprogram obslugi przerwania od SysTick'a
{
	sekunda+=1;
	if(sekunda==czas)
	{
		switch(gap_ton)
		{
			case 0	:		czas=Fuga_W[ton];		// Fuga
									//czas=Kotek_W[ton];		// Kotek
									mod_curr=Tony[Fuga[ton]]/Oktawa[gama];		// Fuga
									//mod_curr=Tony[Kotek[ton]]/Oktawa[gama];			// Kotek
									TPM0->MOD = mod_curr;
									ampl=(int)mod_curr/50;
									TPM0->CONTROLS[3].CnV = ampl;	// Nowa wartość kreująca współczynnik wypełnienia PWM
									TPM0->SC |= TPM_SC_CMOD(1);
									sekunda=0;
									gap_ton=1;
									break;
			case 1	:		TPM0->SC &= ~(TPM_SC_CMOD(3));
									TPM0->CNT=0;
									sekunda=0;
									//czas=Fuga_Gap[ton];		// Fuga
									czas=Kotek_Gap[ton];		// Kotek
									ton+=1;
									//if(ton==16)		// Fuga
									if(ton==18)			// Kotek
										ton=0;
									gap_ton=0;
									break;
		}
	}
}

void PORTA_IRQHandler(void)	// Podprogram obslugi przerwania od klawiszy S2, S3 i S4
{
	uint32_t buf;
	buf=PORTA->ISFR & (S2_MASK | S3_MASK | S4_MASK);

	switch(buf)
	{
		case S2_MASK:	DELAY(10)
									if(!(PTA->PDIR&S2_MASK))		// Minimalizacja drgan zestyków
									{
										if(!(PTA->PDIR&S2_MASK))	// Minimalizacja drgan zestyków (c.d.)
										{
											if(!S2_press)
											{
												S2_press=1;
											}
										}
									}
									break;
		case S3_MASK:	DELAY(10)
									if(!(PTA->PDIR&S3_MASK))		// Minimalizacja drgan zestyków
									{
										if(!(PTA->PDIR&S3_MASK))	// Minimalizacja drgan zestyków (c.d.)
										{
											if(!S3_press)
											{
												S3_press=1;
											}
										}
									}
									break;
		case S4_MASK:	DELAY(10)
									if(!(PTA->PDIR&S4_MASK))		// Minimalizacja drgan zestyków
									{
										if(!(PTA->PDIR&S4_MASK))	// Minimalizacja drgan zestyków (c.d.)
										{
											if(!S4_press)
											{
												S4_press=1;
											}
										}
									}
									break;
		default:			break;
	}	
	PORTA->ISFR |=  S2_MASK | S3_MASK | S4_MASK;	// Kasowanie wszystkich bitów ISF
	NVIC_ClearPendingIRQ(PORTA_IRQn);
}

int main (void) 
{
	Klaw_Init();
	Klaw_S2_4_Int();
	LCD1602_Init();		 // Inicjalizacja LCD
	LCD1602_Backlight(FALSE);
	PWM_Init();				// Inicjalizacja licznika TPM0 (PWM „Low-true pulses”)
	SysTick_Config(SystemCoreClock/DIV_CORE );	// Start licznika SysTick
	
	ampl=(int)mod_curr/50;

  while(1)
	{
		if(S2_press)							// Gama w górę
		{
			gama+=1;
			if(gama==7)
				gama=6;
			S2_press=0;
		}
		if(S3_press)							// Gama w dół
		{
			gama-=1;
			if(gama==(-1))
				gama=0;
			S3_press=0;
		}
		if(S4_press)							// Zatrzymaj/wznów
		{
			on_off=(~on_off);
			if(on_off)
				SysTick_Config(SystemCoreClock/DIV_CORE);
			else
			{
				SysTick_Config(1);
				TPM0->SC &= ~(TPM_SC_CMOD(3));
				TPM0->CNT=0;
			}
			S4_press=0;
		}
	}
}
