/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 5 - Przetwornik C/A - Kształt
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
#define DIV_CORE	10000	// Przerwanie co 0.1ms - dźwięk 500Hz

volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;

float wynik, volt_coeff = 0.01455;
uint8_t on_off=255;
uint16_t temp, slider;
uint16_t dac;
int32_t Sinus[20];
int32_t Trojkat[]={0,409,818,1227,1636,2045,1636,1227,818,409,0,-409,-818,-1227,-1636,-2045,-1636,-1227,-818,-409};
int32_t	Pila[]={0,205,409,614,819,1024,1228,1433,1638,1842,2047,-2047,-1820,-1592,-1365,-1137,-910,-682,-455,-227};
uint8_t n, ksztalt=0;

void SysTick_Handler(void)	// Podprogram obsługi przerwania od SysTick'a
{ 
	switch(ksztalt)
	{
		case 0:	dac=(Sinus[n]*slider)/100+0x0800;					// Przebieg sinusoidalny
						break;
		case 1:	dac=(Trojkat[n]*slider)/100+0x0800;			// Przebieg trójkątny
						break;
		case 2:	dac=(Pila[n]*slider)/100+0x0800;			// Przebieg piłokształtny
						break;
	}
	DAC_Load_Trig(dac);
	n+=1;
	if(n>19)	// Czy wysłano 20 próbek?
		n=0;
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
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	Klaw_Init();	// Inicjalizacja klawiatury
	Klaw_S2_4_Int();	// Klawisze S2, S3 i S4 na przerwaniach
	TSI_Init();					// Inicjalizacja pola dotykowego - Slider
	LCD1602_Init();		 	// Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	sprintf(display,"Volume=%02d",slider);	// Poziom głośności (nastawa P1)
	LCD1602_SetCursor(0,0);
	LCD1602_Print(display);
	LCD1602_Print("%");
	
	LCD1602_SetCursor(0,1);
	wynik = (float)slider*volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
	sprintf(display,"Amplituda=%.2fV",wynik);		// Wyświetlenie wartości amplitudy przebiegu wyjściowego (DAC0)
	LCD1602_Print(display);
	
	DAC_Init();		// Inicjalizacja prztwornika C/A
	for(n=0;n<20;n++)
		Sinus[n]=(sin((double)n*0.314159)*2047.0); // Ładowanie dwudziestu, 12-bitowych próbek funkcji sisnus do tablicy
	n=0;
	SysTick_Config(SystemCoreClock/DIV_CORE);	// Start licznika SysTick
	
	while(1)
	{
		w=TSI_ReadSlider();		// Odczytaj pozycję panelu dotykowego
		if(w!=0)							
		{
			slider=w;
			LCD1602_SetCursor(7,0);
			sprintf(display,"%02d",w);	// Wyświetl aktualną głośność
			LCD1602_Print(display);
			LCD1602_SetCursor(10,1);
			wynik = (float)slider*volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
			sprintf(display,"%.2fV",wynik);		// Wyświetlenie wartości amplitudy przebiegu wyjściowego (DAC0)
			LCD1602_Print(display);
		}
		if(S2_press)							
		{
			ksztalt=0;	// Wybierz sisnus
			S2_press=0;
		}
		if(S3_press)							
		{
			ksztalt=1;	// Wybierz trójkąt
			S3_press=0;
		}
		if(S4_press)
		{
			ksztalt=2;	// Wybierz piłę
			S4_press=0;
		}
	}
}
