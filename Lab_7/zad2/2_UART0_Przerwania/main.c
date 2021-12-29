/*-------------------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - laboratorium
					Lab 7 - Ćwiczenie 2: testowanie odbiornika z przerwaniami
					autor: Mariusz Sokołowski
					wersja: 28.09.2021r.
----------------------------------------------------------------------------*/
					
#include "MKL05Z4.h"
#include "uart0.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char rx_buf[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
char temp;
uint8_t rx_FULL=0;

void UART0_IRQHandler()
{
	if(UART0->S1 & UART0_S1_RDRF_MASK)
	{
		temp=UART0->D;	// Odczyt wartości z bufora odbiornika i skasowanie flagi RDRF
		if(!rx_FULL)
		{
			rx_buf[0] = temp;	// Wysłanie danej do pętli głównej
			rx_FULL=1;
		}
	}
	NVIC_EnableIRQ(UART0_IRQn);
}
int main (void)
{	
	UART0_Init();		// Inicjalizacja portu szeregowego UART0
	while(1)
	{
		if(rx_FULL)		// Czy dana gotowa?
		{
			while(!(UART0->S1 & UART0_S1_TDRE_MASK));	// Czy anadajnik gotowy?
			UART0->D = rx_buf[0];  // Wyślij daną z powrotem do komputera
			rx_FULL=0;	// Dana skonsumowana
		}
	}
}
