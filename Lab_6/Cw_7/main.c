/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 6 - Ćwiczenie 1: wyzwalanie programowe przetwornika A/C - tryb wyzwalania automatycznego
					autor: Mariusz Sokołowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
					
#include "MKL05Z4.h"
#include "ADC.h"
#include "pit.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
uint8_t wynik_ok=0;
float temp = 0;
uint8_t count =0;
float	wynik;


void ADC0_IRQHandler()
{	
	temp = ADC0->R[0];										// Odczyt danej i skasowanie flagi COCO
	if(!wynik_ok)													// Sprawdź, czy wynik skonsumowany przez pętlę główną
	{
		wynik = temp;												// Wyślij nową daną do pętli głównej
		wynik_ok=1;
	}
				

}

int main (void)
{
	uint8_t	kal_error;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	LCD1602_Init();		 // Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_Print("---");				// Ekran kontrolny - nie zniknie, jeśli dalsza część programu nie działa

		
	
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{	
		while(1);									// Klaibracja się nie powiodła
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(26);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 8 i odblokowanie przerwania
	
	while(1)
	{
		if(wynik_ok)
		{
			wynik = 25 - (wynik*adc_volt_coeff-0.716)/0.00162;	// Dostosowanie wyniku do zakresu napięciowego
			
			sprintf(display,"Temp=%.1f%cC",wynik,(char)223); //kod ascii 223 to kropeczka
			LCD1602_SetCursor(0,0);
			LCD1602_Print(display);
			wynik_ok=0;
		}
	}
}
