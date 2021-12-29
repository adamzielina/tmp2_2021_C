/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 6 - Ćwiczenie 6: regulator zmierzchowy
					autor: Mariusz Sokołowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
					
#include "MKL05Z4.h"
#include "ADC.h"
#include "DAC.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

float adc_volt_coeff = ((float)(((float)2.91) /4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
uint8_t wynik_ok=0;
uint16_t temp;
float wynik;

void ADC0_IRQHandler()
{	
	temp = ADC0->R[0];		// Odczyt danej i skasowanie flagi COCO
	if(!wynik_ok)					// Sprawdź, czy wynik skonsumowany przez petlę główną
	{
		wynik = 4095- temp;				// Wyślij wynik do pętli głównej
		wynik_ok=1;
	}
	DAC_Load_Trig(4095-temp);	// Załadowanie nowej danej i wyzwolenie przetwornika C/A
	
}
int main (void)
{
	uint8_t	kal_error;

	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

	LCD1602_Init();		 // Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);

	LCD1602_SetCursor(0,0);
	LCD1602_Print("Uk=");
	
	kal_error=ADC_Init();		// Inicjalizacja przetwornika A/C
	if(kal_error)
	{
		while(1);							// Klaibracja się nie powiodła
	}
	DAC_Init();		// Inicjalizacja przetwornika C/A
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 8 i odblokowanie przerwania
	
	while(1)
	{
		if(wynik_ok)
		{
			wynik = wynik*adc_volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
			sprintf(display,"%.3fV",wynik);
			LCD1602_SetCursor(3,0);
			LCD1602_Print(display);
			wynik_ok=0;
		}
	}
}
