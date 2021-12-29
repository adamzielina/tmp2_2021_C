/*-----------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 5 - Ćwiczenie 5: współpraca przetwornika A/C z prztwornikiem C/A
					autor: Mariusz Sokołowski
					wersja: 07.11.2020r.
------------------------------------------------------------------------------*/
					
#include "MKL05Z4.h"
#include "ADC.h"
#include "DAC.h"
#include "pit.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
uint8_t wynik_ok=0;
uint16_t temp, dac_old;
uint16_t dac=0;
float	wynik, dac_nap;

void PIT_IRQHandler()
{
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;		// Skasuj flagę żądania przerwania
}

void ADC0_IRQHandler()
{	
	temp = ADC0->R[0];		// Odczyt danej i skasowanie flagi COCO
	if(!wynik_ok)					// Sprawdź, czy wynik skonsumowany przez pętlę główną
	{
		wynik = temp;				// Wyślij nową daną do pętli głównej
		dac_old=dac;
		if(dac>4095)
			dac=0;
		else
			dac+=1;
		DAC_Load_Trig(dac);		// Ustawienie nowej wartości prztwornika C/A
		wynik_ok=1;
	}	
}
int main (void)
{
	uint8_t	kal_error;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	LCD1602_Init();		 // Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_Print("---");		// Ekran kontrolny - nie zniknie, jeśli dalsza część programu nie działa
	
	kal_error=ADC_Init();		// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{
		while(1);							// Klaibracja się nie powiodła
	}
	DAC_Init();						// Inicjalizacja prztwornika C/A
	DAC_Load_Trig(dac);		// Wyzwolenie prztwornika C/A wartością początkową
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);		// Odblokowanie przerwania i wybranie kanału nr 8
	PIT_Init();			// Inicjalizacja licznika PIT0
	
	while(1)				// Klaibracja się nie powiodła
	{
		if(wynik_ok)
		{
			wynik = wynik*adc_volt_coeff;				// Dostosowanie wyniku do zakresu napięciowego
			sprintf(display,"A/C=%.4fV",wynik);
			LCD1602_SetCursor(0,0);
			LCD1602_Print(display);
			dac_nap = dac_old*adc_volt_coeff;		// Dostosowanie wartości do zakresu napięciowego
			sprintf(display,"C/A=%.4fV",dac_nap);
			LCD1602_SetCursor(0,1);
			LCD1602_Print(display);
			wynik_ok=0;
		}
	}
}
