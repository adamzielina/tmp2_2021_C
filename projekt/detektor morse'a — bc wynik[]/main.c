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
uint16_t count =0;
float	wynik[10];

int msek = 0;
int sek=0;
int sek3 =0;

int j = 0;

void PIT_IRQHandler()
{
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

	wynik[j] = temp/count;
	j +=1 ;
	if (j== 10)
		j=0;
	wynik_ok =1;
	
	msek += 1;
	if (msek == 10)
		sek =1;
	else if (msek ==30){
		sek3 =1;
		msek = 0;
	}
	
	count = 0;
	temp = 0;
		
}


void ADC0_IRQHandler()
{	

	count++;
	temp += ADC0->R[0];	// Odczyt danej i skasowanie flagi COCO	
	
}



int main (void)
{
	
	uint8_t	kal_error;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	LCD1602_Init();		 // Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_Print("---");				// Ekran kontrolny - nie zniknie, jeśli dalsza część programu nie działa
	PIT_Init();
		
	
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{	
		while(1);									// Klaibracja się nie powiodła
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 8 i odblokowanie przerwania
	
	while(1)
	{
		
		if(wynik_ok) //minelo 100ms
		{
			wynik[j] = wynik[j]*adc_volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
			sprintf(display,"U=%.3fV",wynik[j]);
			LCD1602_SetCursor(0,1);
			LCD1602_Print(display);

			wynik_ok=0;
			
		}
	}
}
