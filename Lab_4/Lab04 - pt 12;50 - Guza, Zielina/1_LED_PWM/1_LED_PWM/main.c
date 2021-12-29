/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 4 - LED-PWM
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

#define MOD_10Hz	65535		// MOD dla częstotliwości 10Hz
#define MOD_100Hz	6553
#define MOD_1000Hz 654		// MOD dla częstotliwości ok. 1002Hz
#define ZEGAR 655360

volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;
float	ampl_v;
float freq;
uint16_t	mod_curr=MOD_10Hz;	// Początkowy MOD dla f=10Hz
uint8_t k_curr=50;					// Początkowy współczynnik wypełnienia K=50%, ustawiany przez pole dotykowe 0-100%
uint16_t	ampl;

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
	uint8_t w;
	
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	Klaw_Init();
	Klaw_S2_4_Int();
	LCD1602_Init();		 // Inicjalizacja LCD
	LCD1602_Backlight(TRUE);
	TSI_Init();				// Inicjalizacja pola dotykowego - Slider
	PWM_Init();				// Inicjalizacja licznika TPM0 (PWM „Low-true pulses”)
	LCD1602_ClearAll();
	ampl_v=3.0*(100.0-k_curr)/100.0;		// Średnie napięcie na wyjściu PWM - zakres od 0V do 3.0V
	sprintf(display,"U=%.2fV",ampl_v);			// Wyświetlenie wartości średniego napięcia sygnału PWM
	LCD1602_Print(display);
	
	LCD1602_SetCursor(9,0);
	sprintf(display,"k=%02d",k_curr);			// Wyświetlenie wartości współczynnikia wypełnienia sygnału PWM
	LCD1602_Print(display);
	LCD1602_Print("%");
	
	TPM0->MOD = mod_curr;		// Początkowa wartość MOD dla f=10Hz
	ampl=((int)mod_curr*k_curr)/100;	// Współczynnik wypełnienia k przeliczony dla aktualnego MOD - realna warotść CnV
	TPM0->CONTROLS[3].CnV = ampl; 	// Sygnał o nowym k poadany na diodę R LED
	freq=(float)ZEGAR/(float)mod_curr;	// Wyliczenie częstotliwości na podstawie wartości zegara i rejestru MOD
	LCD1602_SetCursor(0,1);
	sprintf(display,"f=%.1fHz",freq);		// Wyświetlenie wartośći częstotliwości sygnału wyjściowego PWM
	LCD1602_Print(display);

  while(1)
	{
		w=TSI_ReadSlider();
		if(w!=0)
		{
			k_curr=w;
			ampl_v=3.0*(100.0-k_curr)/100.0;		// Średnie napięcie na wyjściu PWM - zakres od 0V do 3.0V
			LCD1602_SetCursor(2,0);
			LCD1602_Print("     ");
			LCD1602_SetCursor(2,0);
			sprintf(display,"%.2fV",ampl_v);
			LCD1602_Print(display);
			
			LCD1602_SetCursor(11,0);
			sprintf(display,"%02d",k_curr);			// Wyświetlenie wartości współczynnikia wypełnienia sygnału PWM
			LCD1602_Print(display);
			LCD1602_Print("%");
			
			ampl=((int)mod_curr*k_curr)/100;
			TPM0->CONTROLS[3].CnV = ampl;	// Nowa wartość kreująca współczynnik wypełnienia PWM		
		}
		if(S2_press)							// Włącz f=10Hz
		{
			mod_curr=MOD_10Hz;						
			TPM0->MOD = mod_curr;										//	Rejestr MOD=65535 - fwy=10Hz	
			ampl=((int)mod_curr*k_curr)/100;	// Współczynnik wypełnienia k przeliczony dla aktualnego MOD - realna warotść CnV
			TPM0->CONTROLS[3].CnV = ampl; 	// Sygnał o nowym k poadany na diodę R LED
			TPM0->SC |= TPM_SC_CMOD(1);							// Włącz licznik TPM0
			freq=(float)ZEGAR/(float)mod_curr;	// Wyliczenie częstotliwości na podstawie wartości zegara i rejestru MOD
			LCD1602_SetCursor(2,1);
			LCD1602_Print("          ");
			LCD1602_SetCursor(2,1);
			sprintf(display,"%.1fHz",freq);		// Wyświetlenie wartośći częstotliwości sygnału wyjściowego PWM
			LCD1602_Print(display);
			S2_press=0;
		}
		if(S3_press)							// Włącz f=1000Hz
		{
			mod_curr=MOD_1000Hz;						
			TPM0->MOD = mod_curr;										//	Rejestr MOD=41942 - fwy=1000Hz	
			ampl=((int)mod_curr*k_curr)/100;	// Współczynnik wypełnienia k przeliczony dla aktualnego MOD - realna warotść CnV
			TPM0->CONTROLS[3].CnV = ampl; 	// Sygnał o nowym k poadany na diodę R LED
			TPM0->SC |= TPM_SC_CMOD(1);							// Włącz licznik TPM0
			freq=(float)ZEGAR/(float)mod_curr;	// Wyliczenie częstotliwości na podstawie wartości zegara i rejestru MOD
			LCD1602_SetCursor(2,1);
			LCD1602_Print("          ");
			LCD1602_SetCursor(2,1);
			sprintf(display,"%.1fHz",freq);		// Wyświetlenie wartośći częstotliwości sygnału wyjściowego PWM
			LCD1602_Print(display);
			S3_press=0;
		}
		if(S4_press)							// Włącz f=100Hz
		{
			mod_curr=MOD_100Hz;						
			TPM0->MOD = mod_curr;										//	Rejestr MOD=41942 - fwy=1000Hz	
			ampl=((int)mod_curr*k_curr)/100;	// Współczynnik wypełnienia k przeliczony dla aktualnego MOD - realna warotść CnV
			TPM0->CONTROLS[2].CnV = ampl; 	// Sygnał o nowym k poadany na diodę Blue led
			TPM0->SC |= TPM_SC_CMOD(1);							// Włącz licznik TPM0
			freq=(float)ZEGAR/(float)mod_curr;	// Wyliczenie częstotliwości na podstawie wartości zegara i rejestru MOD
			LCD1602_SetCursor(2,1);
			LCD1602_Print("          ");
			LCD1602_SetCursor(2,1);
			sprintf(display,"%.1fHz",freq);		// Wyświetlenie wartośći częstotliwości sygnału wyjściowego PWM
			LCD1602_Print(display);
			S3_press=0;
		}
	}
}
