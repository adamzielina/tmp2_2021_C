/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 3 - Klawiatura-przerwanie
					autor: Mariusz Sokolowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
	
#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include "leds.h"
#include "klaw.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define S3_LIMIT	10	// Limit liczby wcisniec klawisza S3
#define S4_LIMIT	10	// Limit liczby wcisniec klawisza S4

volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"
volatile uint8_t S3_press=0;
volatile uint8_t S4_press=0;
uint8_t S3_nr=0;						// Liczba wcisniec klawisza S3
uint8_t S4_nr=0;						// Liczba wcisniec klawisza S4

void PORTA_IRQHandler(void)	// Podprogram obslugi przerwania od klawiszy S2, S3 i S4
{
	uint32_t buf;
	buf=PORTA->ISFR & (S2_MASK | S3_MASK | S4_MASK);

	switch(buf)
	{
		case S2_MASK:	DELAY(10)
									if(!(PTA->PDIR&S2_MASK))		// Minimalizacja drgan zestyków
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
		case S3_MASK:	DELAY(10)
									if(!(PTA->PDIR&S3_MASK))		// Minimalizacja drgan zestyków
									{
										if(!(PTA->PDIR&S3_MASK))	// Minimalizacja drgan zestyków (c.d.)
										{
											if(!S3_press)
											{
												S3_nr+=1;
												if(S3_nr>S3_LIMIT)
													S3_nr=0;
												S3_press=1;
											}
										}
									}
									break;
		case S4_MASK:	if(!S4_press)
									{
										S4_nr+=1;
										if(S4_nr>S4_LIMIT)
											S4_nr=0;
										S4_press=1;
									}
									break;
		default:			break;
	}	
	PORTA->ISFR |=  S2_MASK | S3_MASK | S4_MASK;	// Kasowanie wszystkich bitów ISF
	NVIC_ClearPendingIRQ(PORTA_IRQn);
}

int main (void) 
{
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	Klaw_Init();								// Inicjalizacja klawiatury
	Klaw_S2_4_Int();						// Klawisze S2, S3 i S4 zglaszaja przerwanie
	LED_Init();									// Inicjalizacja diod LED
	LCD1602_Init();		 					// Inicjalizacja LCD
	LCD1602_Backlight(TRUE);  	// Wlaczenie podswietlenia
	
	LCD1602_ClearAll();					// Wyczysc ekran
	LCD1602_Print("Witam i");		// Wyswietl powitanie (ciag tekstowy)
	LCD1602_SetCursor(0,1);			// Ustaw kursor na poczatku drugiej linii
	LCD1602_Print("o zdrowie pytam.");	// Wyswietl powitanie c.d.
	
	while(PTA->PDIR&S1_MASK);  	// Czy klawisz S1 wcisniety? (oczekiwanie na wcisniecie klawisza)
	
	LCD1602_ClearAll();					// Wyczysc ekran
	
	sprintf(display,"S3=%02d",S3_nr);	// Wyswietl stan poczatkowy liczby wcisniec klawisza S3
	LCD1602_Print(display);
	
	sprintf(display,"  S4=%02d",S4_nr);	// Wyswietl stan poczatkowy liczby wcisniec klawisza S3
	LCD1602_Print(display);
	
	
  while(1)		// Poczatek petli glównej
	{
		if(S2_press)	// Czy klawisz S2 wcisniety?
		{
			PTB->PTOR|=LED_R_MASK;				// Jesli tak, to zmien na stan przeciwny swiecenie diody czerwonej
			S2_press=0;
		}
		if(S3_press)							// Wyswietl ilosc wzisniec klawisza S3
		{
			LCD1602_SetCursor(3,0);
			sprintf(display,"%02d",S3_nr);
			LCD1602_Print(display);
			S3_press=0;
		}
		if(S4_press)							// Wyswietl ilosc wzisniec klawisza S3
		{
			LCD1602_SetCursor(10,0);
			sprintf(display,"%02d",S4_nr);
			LCD1602_Print(display);
			S4_press=0;
		}
		
	}
}
