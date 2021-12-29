/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 5 - Przetwornik C/A - DDS
					autor: Mariusz Sokołowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
	
#include "MKL05Z4.h"
#include "DAC.h"
#include "tsi.h"
#include "klaw.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define DIV_CORE	8192	// Przerwanie co 0.120ms - fclk=8192Hz df=8Hz
#define MASKA_10BIT	0x03FF

volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;
float wynik, volt_coeff = 0.01455;

uint8_t on_off=255;
int16_t temp;
uint16_t slider;
uint16_t dac;
int16_t Sinus[1024];
uint16_t faza, mod, freq, df;

void SysTick_Handler(void)	// Podprogram obslugi przerwania od SysTick'a
{ 
	dac=(Sinus[faza]/100)*slider+0x0800;					// Przebieg sinusoidalny
	DAC_Load_Trig(dac);
	faza+=mod;		// faza - generator cyfrowej fazy
	faza&=MASKA_10BIT;	// rejestr sterujący przetwornikiem, liczący modulo 1024 (N=10 bitów)
}

void PORTA_IRQHandler(void)	// Podprogram obslugi przerwania od klawiszy S2, S3 i S4
{
	uint32_t buf;
	buf=PORTA->ISFR & (S2_MASK | S3_MASK | S4_MASK);

	switch(buf)
	{
		case S2_MASK:	if(!(PTA->PDIR&S2_MASK))		// Minimalizacja drgan zestyków
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
		case S3_MASK:	if(!(PTA->PDIR&S3_MASK))		// Minimalizacja drgan zestyków
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
		case S4_MASK:	if(!(PTA->PDIR&S4_MASK))		// Minimalizacja drgan zestyków
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
	slider=50;
	df=DIV_CORE/MASKA_10BIT;	// Rozdzielczość generatora delta fs
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	Klaw_Init();
	Klaw_S2_4_Int();
	TSI_Init();					// Inicjalizacja pola dotykowego - Slider
	LCD1602_Init();		 	// Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);

	wynik = (float)slider*volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
	sprintf(display,"Amplituda=%.2fV",wynik);		// Wyświetlenie wartości amplitudy przebiegu wyjściowego (DAC0)
	LCD1602_Print(display);
	
	DAC_Init();		// Inicjalizacja prztwornika C/A
	for(faza=0;faza<1024;faza++)
		Sinus[faza]=(sin((double)faza*0.0061359231515)*2047.0); // Ładowanie 1024-ech sztuk, 12-bitowych próbek funkcji sisnus do tablicy
	faza=0;		// Ustawienie wartości początkowych generatora cyfrowej fazy i modulatora fazy
	mod=1;
	freq=mod*df;
	LCD1602_SetCursor(0,1);
	sprintf(display,"Freq=%04dHz",freq);
	LCD1602_Print(display);
	SysTick_Config(SystemCoreClock/DIV_CORE);	// Start licznika SysTick (generatora DDS)
	
	while(1)
	{
		w=TSI_ReadSlider();		// Odczytaj pozycję panelu dotykowego
		if(w!=0)							
		{
			slider=w;
			LCD1602_SetCursor(10,0);
			wynik = (float)slider*volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
			sprintf(display,"%.2fV",wynik);		// Wyświetlenie wartości amplitudy przebiegu wyjściowego (DAC0)
			LCD1602_Print(display);
		}
		if(S2_press)	// Zwiększ częstotliwość o 8Hz (mod - modulator fazy)							
		{
			temp=mod;
			temp+=1;
			if(temp>510)
				temp=510;
			mod=temp;
			freq=mod*df;
			LCD1602_SetCursor(5,1);
			sprintf(display,"%04dHz",freq);	// Wyświetl aktualną częstotliwość
			LCD1602_Print(display);
			S2_press=0;
		}
		if(S3_press)	// Zmniejsz częstotliwość o 8Hz (mod - modulator fazy)		
		{
			temp=mod;
			temp-=1;
			if(temp<1)
				temp=1;
			mod=temp;
			freq=mod*df;
			LCD1602_SetCursor(5,1);
			sprintf(display,"%04dHz",freq);	// Wyświetl aktualną częstotliwość
			LCD1602_Print(display);
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
			}
			S4_press=0;
		}
	}
}
