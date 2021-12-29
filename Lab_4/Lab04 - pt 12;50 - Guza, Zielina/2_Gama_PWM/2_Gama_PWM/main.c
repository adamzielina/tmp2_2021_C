/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 4 - TPM/PWM - generator dźwięków temperowanych (8 oktaw)
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
#define MOD_MIN 156
#define ZEGAR 1310720

volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;
float	ampl_v;
float freq;
uint16_t	mod_curr=MOD_MAX;
uint16_t	ampl;
uint16_t Tony[]={40082, 35701, 31813, 30027, 26755, 23830, 21229, 20041};
uint16_t	Oktawa[]={1, 2, 4, 8, 16, 32, 64, 128};
uint8_t	ton=0;
int8_t	gama=0;
uint8_t k_curr=50;

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
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	Klaw_Init();
	Klaw_S2_4_Int();
	LCD1602_Init();		 // Inicjalizacja LCD
	LCD1602_Backlight(TRUE);
	TSI_Init();				// Inicjalizacja pola dotykowego - Slider
	PWM_Init();				// Inicjalizacja licznika TPM0 (PWM „Low-true pulses”)
	LCD1602_ClearAll();
	ampl_v=3.0*(100.0-k_curr)/100.0;
	sprintf(display,"U=%.2fV",ampl_v);
	LCD1602_Print(display);
	
	TPM0->MOD = mod_curr;
	ampl=((int)mod_curr*k_curr)/100;
	TPM0->CONTROLS[3].CnV = ampl;	// Nowa wartość kreująca współczynnik wypełnienia PWM
	freq=(float)ZEGAR/(float)mod_curr;
	LCD1602_SetCursor(0,1);
	sprintf(display,"f=%.1fHz",freq);
	LCD1602_Print(display);

  while(1)
	{
		if(S2_press)							// Zwiększ częstotliwość
		{
			gama+=1;
			if(gama==8)
				gama=7;
			mod_curr+=1;
			S2_press=0;
		}
		if(S3_press)							// Zmniejsz częstotliwość
		{
			gama-=1;
			if(gama==(-1))
				gama=0;
			mod_curr-=1;
			S3_press=0;
		}
		if(S4_press)							// Zmniejsz częstotliwość
		{
			mod_curr=Tony[ton]/Oktawa[gama];
			TPM0->MOD = mod_curr;
			ton+=1;
			if(ton==8)
				ton=0;
			ampl=((int)mod_curr*k_curr)/100;
			TPM0->CONTROLS[3].CnV = ampl;	// Nowa wartość kreująca współczynnik wypełnienia PWM
			freq=(float)ZEGAR/(float)mod_curr;
			LCD1602_SetCursor(2,1);
			LCD1602_Print("         ");
			LCD1602_SetCursor(2,1);
			sprintf(display,"%.1fHz",freq);
			LCD1602_Print(display);
			S4_press=0;
		}
	}
}
