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
					
//pliki nagłówkowe 					
#include "MKL05Z4.h"
#include "ADC.h"
#include "pit.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "klaw.h"
/* tworzymy zbiór 
kodujemy na 8bitowej liczbie, każdy znak alfabetu, 
cyfry oraz znaki specjalne
		. -> 01
		- -> 10
		ZS -> 11
przykładowo:
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

float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
uint8_t wynik_ok = 0;
float temp = 0;
uint16_t count = 0;
float	wynik[20];
float wyn;
int msek = 0;
int sek = 0;
int sek2 = 0;
volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"


void PIT_IRQHandler() { //handler licznika PIT

	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

	
	wyn = temp/count;
	wyn = wyn*adc_volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego


	wynik_ok =1;
	
	msek += 1;
	if ( msek ==10)
		sek = 1;
	
	else if ( msek ==20 ){
		
		sek = 1;
		sek2 =1;
		msek = 0;
		
	}
	
	count = 0;
	temp = 0;
		
}

int cycles = 0;

void SysTick_Handler(void)	{// Podprogram obslugi przerwania od SysTick'a
 
	cycles++;
	//400ms underflow  0 < x < 4 -> .
	//			underflow 4 do 5 -> -

}



void ADC0_IRQHandler() { // handler przetwornika ADC
	
	count++;
	temp += ADC0->R[0];	// Odczyt danej i skasowanie flagi COCO	
	
}



void PORTA_IRQHandler(void) {	// Podprogram obslugi przerwania od klawiszy S2, S3 i S4

	
	if(!(PTA->PDIR&S2_MASK))	{	// Minimalizacja drgan zestyków
									
		if(!(PTA->PDIR&S2_MASK)) {	// Minimalizacja drgan zestyków (c.d.)
										
			if(!S2_press) {
				S2_press=1;
			}							
									
		}
	}
								
							
	PORTA->ISFR |=  S2_MASK; 	// Kasowanie wszystkich bitów ISF
	NVIC_ClearPendingIRQ(PORTA_IRQn);
			
}
						





int main (void) {
	
	uint8_t	kal_error;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	LCD1602_Init();		 // Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_Print("---");				// Ekran kontrolny - nie zniknie, jeśli dalsza część programu nie działa
	
	PIT_Init();
	
	Klaw_Init();								// Inicjalizacja klawiatury
	Klaw_S2_4_Int();						// Klawisze S2, S3 i S4 zglaszaja przerwanie
	
	/*zmienne do mierzenia napiecia ref*/
	float wyn_temp=0;
	int count_conf = 0;
	float ref;
	
	int down = 0;
	int up = 0;
	
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{	
		while(1);									// Klaibracja się nie powiodła
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 8 i odblokowanie przerwania
	
	SysTick_Config( 4 * SystemCoreClock/10 ); //liczy do 400ms maks dla 24 bitowego systicka przy clocku 40Mhz(0 - 16 777 215)
	
	while(1) {
		
		/*konfiguracja napiecia referencyjnego do tła*/
		if(S2_press) {
			msek = 0;
			sek2 = 0;
			LCD1602_ClearAll();					// Wyczysc ekran
			LCD1602_SetCursor(0,0);
			sprintf(display,"Konfiguracja");
			LCD1602_Print(display);
			LCD1602_SetCursor(0,1);
			sprintf(display,"napiecia ref");
			LCD1602_Print(display);		
			 //98 9700
				while(!sek2) {
			
					count_conf++;
					wyn_temp += wyn;
				
				}
			
			LCD1602_ClearAll();					// Wyczysc ekran
			ref = wyn_temp/count_conf;
			LCD1602_SetCursor(0,0);
			sprintf(display,"Uref=%f",ref);
			LCD1602_Print(display);
			S2_press = 0;
			sek2=0;
			wyn_temp = 0;
			count_conf = 0;
		}
						
			if(wynik_ok) { //minelo 100ms
			
				if (wyn - ref > 0.20) {
					
					if(down) {
						
						SysTick->VAL = 0;
						cycles = 0;
						down = 0;
						
					}
					up =1;
				}
				
				else {
					
						if(up) {
							
							if (cycles > 0 && cycles < 4) {
								
								LCD1602_SetCursor(0,1);
								sprintf(display,".");
								LCD1602_Print(display);
							
							}
								
							else if (cycles > 3 && cycles < 7) {
								
								LCD1602_SetCursor(1,1);
								sprintf(display,"-");
								LCD1602_Print(display);
									
							}
							
						}	
							
							up = 0;
							
					}
						
					down = 1;
					wynik_ok = 0;
				}
				
				
			LCD1602_SetCursor(5,1);
			sprintf(display,"U=%.6fV",wyn);
			LCD1602_Print(display);
			

			}
		
		}