/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 5 - Przetwornik C/A - LED
					autor: Mariusz Sokołowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
	
#include "MKL05Z4.h"
#include "DAC.h"
#include "tsi.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


double volt_coeff = 0.0291;
float	wynik;
uint16_t temp, slider=50;
uint16_t dac;


int main (void)
{
	uint8_t w;
	slider=50;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	
	TSI_Init();					// Inicjalizacja pola dotykowego - Slider
	LCD1602_Init();		 	// Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	
	DAC_Init();		// Inicjalizacja prztwornika C/A
	dac=41*slider;	// Rozszerzenie zakresu pola dotykowego do 12 bitów
	DAC_Load_Trig(dac);	// Wyzwolenie prztwornika C/A wartością początkową
	wynik = (double)slider*volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
	sprintf(display,"U=%.2fV",wynik);		// Wyświetlenie wartości napięcia wyjściowego przetwornika DAC0
	LCD1602_Print(display);
	
	while(1)
	{
			w=TSI_ReadSlider();
			if(w!=0)
			{
				slider=w;
				dac=41*slider;	// Rozszerzenie zakresu pola dotykowego do 12 bitów
				DAC_Load_Trig(dac);		// Ustawienie nowej wartości prztwornika C/A
				wynik = (double)slider*volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
				sprintf(display,"%.2fV",wynik);		// Wyświetlenie wartości napięcia wyjściowego przetwornika DAC0
				LCD1602_SetCursor(2,0);
				LCD1602_Print(display);
			}
	}
}
