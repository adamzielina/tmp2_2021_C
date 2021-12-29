/*-------------------------------------------------------------------------
					Projekt - odbiornik kodu Morse'a
					autorzy:
					-Adam Zielina
					-Jakub Guza
					przedmiot:
					-Technika Mikroprocesorowa 2
					prowadzacy:
					-Mariusz Sokolowski
----------------------------------------------------------------------------*/
					
//pliki nag³ówkowe 					
#include "MKL05Z4.h"
#include "ADC.h"
#include "pit.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* tworzymy zbiór 
kodujemy na 16bitowej liczbie, ka¿dy znak alfabetu, 
cyfry oraz znaki specjalne
		. -> 01
		- -> 10
		ZS -> 11
przyk³adowo:
		.- -> A
		0110110000000000 
*/

#define a 0b0110110000000000 
#define b 0b1001010111000000
#define c 0b1001100111000000
#define d 0b1001011100000000
#define e 0b0111000000000000
#define f 0b0101100111000000
#define g 0b1010011100000000
#define h 0b0101010111000000
#define i 0b0101110000000000
#define j 0b0110101011000000
#define k 0b1001101100000000
#define l 0b0110010111000000
#define m 0b1010110000000000
#define n 0b1001110000000000
#define o 0b1010101100000000
#define p 0b0110100111000000
#define q 0b1010011011000000
#define r 0b0110011100000000
#define s 0b0101011100000000
#define t 0b1011000000000000
#define u 0b0101101100000000
#define v 0b0101011011000000
#define w 0b0110101100000000
#define x 0b1001011011000000
#define y 0b1001101011000000
#define z 0b1010010111000000

float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Wspó³czynnik korekcji wyniku, w stosunku do napiêcia referencyjnego przetwornika
uint8_t wynik_ok=0;
float temp = 0;
uint16_t count =0;
float	wynik[20];

int msek = 0;
int sek=0;
int sek2 =0;

int J = 0;

uint32_t tester1;

void PIT_IRQHandler() //co 100ms przerwanie pit
{
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

/*	wynik[J] = temp/count;
	wynik[J] = wynik[J]*adc_volt_coeff;		// Dostosowanie wyniku do zakresu napiêciowego

	J+=1;
	
	if (J == 20)
		J=0;
	
	wynik_ok =1;
	
		 

	*/
	msek += 1;	//okresla miniecie 10 i 30s
	if (msek == 100){
		  sek =1;

	}
	else if (msek ==101){
		sek2 =1;
		msek = 0;
	}
	
	//count = 0;
	//temp = 0;
		
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
	int K = 0;	
	LCD1602_Init();		 // Inicjalizacja wyœwietlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_Print("---");				// Ekran kontrolny - nie zniknie, jeœli dalsza czêœæ programu nie dzia³a
	PIT_Init();
		
	
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{	
		while(1);									// Klaibracja siê nie powiod³a
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 8 i odblokowanie przerwania
	tester1= PIT->CHANNEL[0].CVAL;
	while(1)
	{

		
		
		
		if(sek) //minelo 1000ms
		{
			uint32_t tester2 = PIT->CHANNEL[0].CVAL;
			uint32_t tester12 = tester1-tester2;// 
			sprintf(display,"%d-%d",tester1 , tester2);
			LCD1602_SetCursor(0,0);
			LCD1602_Print(display);			
			
			
			
			/*for(int I = 0; I<20; I+=2) {
				if (fabs(wynik[I] - wynik[K]) > 0.05 ) {
					LCD1602_SetCursor(0,1);
					LCD1602_Print(".");
					
					
				}


			
			}
			wynik_ok=0;*/
			sek=0;
		}
	}
}