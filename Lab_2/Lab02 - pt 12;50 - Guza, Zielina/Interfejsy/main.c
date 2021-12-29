/*-------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 2 - Interfejsy uzytkownika
					autor: Mariusz Sokolowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
	
#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include "tsi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define S1_MASK	(1<<9)		// Maska dla klawisza S1
#define S2_MASK	(1<<10)		// Maska dla klawisza S2
#define S3_MASK	(1<<11)		// Maska dla klawisza S3
#define S4_MASK	(1<<12)   // Maska dla klawisza S4
#define LED_R		(1<<8)		// Maska dla diody czerwonej (R)
#define LED_G		(1<<9)
#define LED_B		(1<<10)


void LED_Init(void)
{
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;      // Wlaczenie portu B
	PORTB->PCR[8] |= PORT_PCR_MUX(1);
	PORTB->PCR[9] |= PORT_PCR_MUX(1);	
	PORTB->PCR[10] |= PORT_PCR_MUX(1);
	PTB->PDDR |= (1<<8)|(1<<9)|(1<<10);	// Ustaw na 1 bity 8, 9 i 10 – rola jako wyjscia
	PTB->PDOR|= (1<<8)|(1<<9)|(1<<10);	// Zgas wszystkie diody
}

void Klaw_Init(void)
{
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;		// Wlaczenie portu A
	PORTA->PCR[9] |= PORT_PCR_MUX(1);
	PORTA->PCR[10] |= PORT_PCR_MUX(1);
	PORTA->PCR[11] |= PORT_PCR_MUX(1);
	PORTA->PCR[12] |= PORT_PCR_MUX(1);
	PORTA->PCR[9] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[10] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[11] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA->PCR[12] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}


int main (void) 
{
	uint8_t w=0;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	Klaw_Init();
	LED_Init();
	LCD1602_Init();		 					// Inicjalizacja LCD
	LCD1602_Backlight(TRUE);  	// Wlaczenie podswietlenia
	TSI_Init();									// Inicjalizacja panelu dotykowego
	
	LCD1602_ClearAll();					// Wyczysc ekran
	LCD1602_Print("Witam i");		// Wyswietl powitanie (ciag tekstowy)
	LCD1602_SetCursor(0,1);			// Ustaw kursor na poczatku drugiej linii
	LCD1602_Print("o zdrowie pytam.");	// Wyswietl powitanie c.d.
	
	while(PTA->PDIR&S1_MASK);  	// Czy klawisz S1 wcisniety? (oczekiwanie na wcisniecie klawisza)
	
	LCD1602_ClearAll();					// Wyczysc ekran
	
	LCD1602_SetCursor(0,0);			// Wyswietl stan poczatkowy pola dotykowego
	LCD1602_Print("--");
	
  while(1)		// Poczatek petli glównej
	{
		w=TSI_ReadSlider();		// Odczytaj pozycje panelu dotykowego
		if(w!=0)							// Wyswietl pozycje palca, jezeli dotknieto pola
		{
			LCD1602_SetCursor(0,0);
			sprintf(display,"%02d",w);
			LCD1602_Print(display);
		}
		if(!(PTA->PDIR&S2_MASK))	// Czy klawisz S2 wcisniety?
		{
			PTB->PTOR|=LED_R;				// Jesli tak, to zmien na stan przeciwny swiecenie diody czerwonej
			DELAY(100)							// Opoznienie na drgania zestykow klawiatury
		}
		if(!(PTA->PDIR&S3_MASK))	// Czy klawisz S3 wcisniety?
		{
			PTB->PTOR|=LED_G;				// Jesli tak, to zmien na stan przeciwny swiecenie diody czerwonej
			DELAY(100)							// Opoznienie na drgania zestykow klawiatury
		}
		if(!(PTA->PDIR&S4_MASK))	// Czy klawisz S4 wcisniety?
		{
			PTB->PTOR|=LED_B;				// Jesli tak, to zmien na stan przeciwny swiecenie diody czerwonej
			DELAY(100)							// Opoznienie na drgania zestykow klawiatury
		}
	}
}