/*-------------------------------------------------------------------------
					Projekt - odbiornik świetlny kodu Morse'a
					autorzy:
					-Adam Zielina
					-Jakub Guza
					przedmiot:
					-Technika Mikroprocesorowa 2
					prowadzacy:
					-Mariusz Sokolowski
----------------------------------------------------------------------------*/
// uklad: MKL05Z4
// peryferia: LCD , klawiatura klawiszowa , analogowy czujnik swiatla	ALS-PT19				
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
kodujemy na 16bitowej liczbie, każdy znak alfabetu, 
cyfry oraz znaki specjalne
		. -> 10
		- -> 01
przykładowo:
		.- -> A
		0000000000000110 
		kodujemy od prawej strony aby interpretacja liczbowa byla mniejsza i mozna sie bylo odwolywac indeksowo 
*/


char alfabet[171] = {0x20,'t','e',0x20,0x20,'m','a',0x20,0x20,'n','i',0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,'o','w',0x20,
	0x20,'k','u',0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,'g','r',0x20,0x20,'d','s',0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,'j',0x20,0x20,'y',0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,'q',0x20,0x20,0x20,'x','v',0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,'p',0x20,0x20,'c','f',0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,'z','l',0x20,0x20,'b','h'};

float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
uint8_t wynik_ok = 0;
float temp = 0;
uint16_t count = 0;
float wyn;
int msek = 0;
int sek = 0;
int sek2 = 0;

volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"

int cycles_up = 0; //zmienna zliczająca ilość cykli systicka co 400ms w przerwaniu dla stanu swiecenia
int cycles_down = 0; // zmienna zliczająca ilość cykli systicka co 400ms w przerwaniu dla stanu nieswiecenia

void PIT_IRQHandler() { //handler licznika PIT co 100ms

	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

	wyn = temp/count;						//srednia probek ze 100ms
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



void SysTick_Handler(void)	{// Podprogram obslugi przerwania od SysTick'a
 
	cycles_up++; //zmienna która liczy ile razy minęło 400ms SysTick'a
	cycles_down++;
}



void ADC0_IRQHandler() { // handler przetwornika ADC
	
	count++;
	temp += ADC0->R[0];	// Odczyt danej i skasowanie flagi COCO	
	
}



void PORTA_IRQHandler(void) {	// Obsluga przerwania dla klawisza S2

	
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
	LCD1602_SetCursor(2,0);
	LCD1602_Print("skonfiguruj");				// Ekran kontrolny - nie zniknie, jeśli dalsza część programu nie działa
	LCD1602_SetCursor(2,1);
	LCD1602_Print("wcisnij S2");	
	PIT_Init();
	
	Klaw_Init();								// Inicjalizacja klawiatury
	Klaw_S2_4_Int();						// Klawisze S2, S3 i S4 zglaszaja przerwanie
	
	/*zmienne do mierzenia napiecia ref*/
	float wyn_temp=0;
	int count_conf = 0;
	float ref = -1;
	
	/*zmienne do wykrywania zmiany poprzedniego stanu*/
	int down = 0;
	int up = 0;
	
	/*zmienne do wyniku detekcji */
	uint16_t znak = 0;	
	int miejsce = 0;
	char wynik[50];
	int plc=0;
	
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{	
		while(1);									// Klaibracja się nie powiodła
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 8 i odblokowanie przerwania
	
	SysTick_Config( 4 * SystemCoreClock/10 ); //liczy do 400ms maks dla 24 bitowego systicka przy clocku 40Mhz(0 - 16 777 215)
	int K=0;
	
	
	while(1) {

		/*konfiguracja napiecia referencyjnego do tła*/
		if(S2_press) {
			
			LCD1602_ClearAll();					// Wyczysc ekran
			LCD1602_SetCursor(2,0);
			LCD1602_Print("konfiguruje");
			LCD1602_SetCursor(2,1);
			LCD1602_Print("napiecie ref");		
			
			msek = 0;	//wyzerowanie zliczania do 2 sekund
			sek2 = 0;
			
			while(!sek2) { //zbieranie próbek przez 2 sek, aby je uśrednić
			
				count_conf++;
				wyn_temp += wyn;
				
			}
			
			LCD1602_ClearAll();					// Wyczysc ekran
			ref = wyn_temp/count_conf;
			LCD1602_SetCursor(2,0);
			sprintf(display,"Uref=%.4fV",ref);
			LCD1602_Print(display);
			
			S2_press = 0;
			sek2=0;
			wyn_temp = 0;
			count_conf = 0;
			
		}
			
		/* detekcja swiecenia , odbieranie slowa	*/
		
		if(wynik_ok && ref != -1) { //minelo 100ms i skonfigurowano
			
			if (wyn - ref > 0.20) { //porownanie aktualnej warosci z referencyjna . brak zmiany natezenia swiatla roznica ok. 0 do elsa
															 //zaswiecenie powstaje roznica >0 to nas interesuje , zacienienie roznica <0 wejdzie do elsa
		
			/*wykona sie jesli byl w stanie niskim (nie swiecil) i przeszedl do wysokiego (swiecenie)*/
				if(down) { 
					
					if (cycles_down > 4 && cycles_down < 11) {		//przerwa 1200 - 1999ms (3,4) koniec litery
	
						wynik[plc] = alfabet[znak];
						plc++;
						
						znak=0;
						miejsce=0;
					}
								
					else if (cycles_down > 10 && cycles_down < 16) { //przerwa 2000 -2799ms (5,6)  koniec slowa
														
						wynik[plc] = alfabet[znak];
						plc++;
						wynik[plc] = 0x20;
						plc++;
						
						znak=0;
						miejsce=0;				
					}					
						
					SysTick->VAL = 0;		//start liczenia na poczatku stanu wysokiego
					cycles_up = 0;
					down = 0;
						
				}
				/*wykona sie w kazdym stanie wysokim	*/
				LCD1602_SetCursor(15,0);
				sprintf(display,"%d",cycles_up);
				LCD1602_Print(display);	
				
				up =1;
					
			}
				
			else {
				
			/*wykona sie jesli byl w stanie wysokim i przeszedl do niskiego*/					
				if(up) {
					
					/*odczyt wskazan licznika , ile czasu trwal stan wysoki */
					if (cycles_up > 0 && cycles_up < 6) {		//swiecilo sie 400 - 1199ms (1,2) kropka 10
								
					/*	LCD1602_SetCursor(K,1);
						sprintf(display,".");
						LCD1602_Print(display);*/
						
						znak &=  ~(1<<miejsce);					
						miejsce ++;
						znak |=  1<< miejsce;
						miejsce ++;
	
					}
								
					else if (cycles_up > 5 && cycles_up < 11) { //swiecilo sie 1200 -1999ms (3,4)  kreska 01
									
					/*	LCD1602_SetCursor(K,1);
						sprintf(display,"-");
						LCD1602_Print(display);*/
						
						znak |=  1<< miejsce;					
						miejsce ++;
						znak &=  ~(1<<miejsce);		
						miejsce ++;						
									
					}
						
					else if (cycles_up == 0) { //zabezpieczenie przed zmiana mniejsza niz 400ms

						K--;
					}
					
					SysTick->VAL = 0;		//start liczenia na poczatku stanu niskiego
					cycles_down = 0;
					
					
					K++;
					up = 0;
				
				}
				/*wykona sie w kazdym stanie niskim	*/		
				LCD1602_SetCursor(15,1);
				sprintf(display,"%d",cycles_down);
				LCD1602_Print(display);	
				
				down = 1;		
				
			} 
	/*		LCD1602_SetCursor(5,1);
			sprintf(display,"U=%.4fV",wyn);
			LCD1602_Print(display);			
			wynik_ok = 0;*/
						
			wynik_ok = 0;
					
			LCD1602_SetCursor(0,1);
			
			
			LCD1602_Print(wynik);
		}
	

	}
		
}
