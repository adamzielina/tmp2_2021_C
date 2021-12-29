/*-------------------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 7 - Ćwiczenie 1: nawiązywanie połączenia z komputerem za pomocą portu szeregowego UART0
					autor: Mariusz Sokołowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
					
#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "uart0.h"
#include "i2c.h"
#include "lcd1602.h"
#include "tsi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pit.h"
#include "ADC.h"
float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
uint8_t wynik_ok=0;
float temp = 0;
uint16_t count =0;
float	wynik;


void PIT_IRQHandler()
{
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
	wynik = temp/count;
	wynik_ok =1;
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
	uint32_t baud_rate, i=0;
	uint8_t w=0,licznik=0;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	LCD1602_Init();
	LCD1602_Backlight(TRUE);
	LCD1602_Print("Wybierz BR");
	UART0_Init();		// Inicjalizacja portu szeregowego UART0
	TSI_Init();
	PIT_Init();
	uint8_t	kal_error;
	
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{	
		while(1);									// Klaibracja się nie powiodła
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 8 i odblokowanie przerwania
	
	
	
	while(w==0)
		w=TSI_ReadSlider();		// Ustaw BR dla UART0, dotykaąc pole: 0<1/3 - 9600, 1/3<2/3 - 28800,2/3< - 230400
	
	UART0->C2 &= ~(UART0_C2_TE_MASK | UART0_C2_RE_MASK );		//Blokada nadajnika i o dbiornika
	
	if(w<33)
	{
		UART0->BDH = 1;			//Dla CLOCK_SETUP=0 BR=9600
		UART0->BDL =17;			//Dla CLOCK_SETUP=0 BR=9600
		baud_rate=9600;
	}
	if((w>33)&&(w<66))
	{
		UART0->BDH = 0;			//Dla CLOCK_SETUP=0 BR=28800
		UART0->BDL =91;			//Dla CLOCK_SETUP=0 BR=28800
		baud_rate=28800;
	}
	if(w>66)
	{
		UART0->BDH = 0;			//Dla CLOCK_SETUP=0 BR=230400
		UART0->BDL =11;			//Dla CLOCK_SETUP=0 BR=230400
		baud_rate=230400;
	}
	sprintf(display,"BR=%d b/s",baud_rate);	// Wyświetlenie aktualnejj wartości BR
	LCD1602_SetCursor(0,0);
	LCD1602_Print(display);
	LCD1602_SetCursor(0,1);
	LCD1602_Print("Ustaw BR w PC...");
	DELAY(500)
	w=0;
	while(w==0)
		w=TSI_ReadSlider();		// Ustaw BR w komputerze i dotknij pole dotykowe
	
	LCD1602_SetCursor(0,1);
	LCD1602_Print("Transmisja...   ");
	UART0->C2 |= UART0_C2_TE_MASK;		//Włącz nadajnik
	
	while(1)
	{
		if(wynik_ok)
		{
			wynik = wynik*adc_volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego
			sprintf(display,"U=%.4fV%c",wynik,0xd);
			LCD1602_SetCursor(0,0);
			
			
			for(i=0;display[i]!=0;i++)
			{
				while(!(UART0->S1 & UART0_S1_TDRE_MASK));	// Czy nadajnik jest pusty?
				UART0->D = display[i];		// Wyślij aktualną wartość licznika
			}
			
			//wyswietl na LCD
			sprintf(display,"U=%.4fV",wynik);
			LCD1602_Print(display);
			wynik_ok=0;
	}
}

}
