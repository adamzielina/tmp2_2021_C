/**----------------------------------------------------------------------------------
 *
 *				@Projekt:
 *          -odbiornik kodu Morse'a, nadajnikiem jest latarka, komunikacja UART,
 *           odtwarzanie wiadomosci za pomoca glosnika
 *				@autorzy:
 *					-Adam Zielina
 *					-Jakub Guza
 *				@przedmiot:
 *				  -Technika Mikroprocesorowa 2
 *				@prowadzacy:
 *					-Mariusz Sokolowski
 *        @uklad:
 *          -MKL05Z4
 *				@peryferia:
 *          -LCD, klawiatura klawiszowa, glosnik, analogowy czujnik swiatla	ALS-PT19
 *
 *-----------------------------------------------------------------------------------                                                               */
	
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "MKL05Z4.h"
#include "ADC.h"
#include "pit.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include "klaw.h"
#include "uart0.h"
#include "tpm_pcm.h"
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

	//PIT i ADC
float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
uint8_t wynik_ok = 0;
float temp = 0;
uint16_t count = 0;
float wyn;
int msek = 0;
int sek2 = 0;

	//PORT_A
volatile uint8_t S2_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"
volatile uint8_t S3_press=0;	// "1" - klawisz zostal wcisniety "0" - klawisz "skonsumowany"

	//systick
int cycles_up = 0; //zmienna zliczająca ilość cykli systicka co 400ms w przerwaniu dla stanu swiecenia
int cycles_down = 0; // zmienna zliczająca ilość cykli systicka co 400ms w przerwaniu dla stanu nieswiecenia
	
	//UART0
int var =0;
char rx_buf[50];
uint8_t rx_buf_pos=0;
int enter =0;	

void PIT_IRQHandler() { //handler licznika PIT co 100ms

	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

	wyn = temp/count;						//srednia probek ze 100ms
	wyn = wyn*adc_volt_coeff;		// Dostosowanie wyniku do zakresu napięciowego


	wynik_ok =1;
	
	msek += 1;
	if ( msek ==20 ){
		
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



void PORTA_IRQHandler(void) {	// Obsluga przerwania dla klawisza S2 i s3

	uint32_t buf;
	buf=PORTA->ISFR & (S2_MASK | S3_MASK);
		
	switch(buf){
		
			case S2_MASK:	
				if(!(PTA->PDIR&S2_MASK))	{	// Minimalizacja drgan zestyków
												
					if(!(PTA->PDIR&S2_MASK)) {	// Minimalizacja drgan zestyków (c.d.)
													
						if(!S2_press) {
							S2_press=1;
						}													
					}
				}
				break;
				
			case S3_MASK:
				if(!(PTA->PDIR&S3_MASK))	{	// Minimalizacja drgan zestyków
												
					if(!(PTA->PDIR&S3_MASK)) {	// Minimalizacja drgan zestyków (c.d.)
													
						if(!S3_press) {
							S3_press=1;
						}													
					}
				}
				break;
	}	
								
	PORTA->ISFR |=  S2_MASK|S3_MASK; 	// Kasowanie wszystkich bitów ISF
	NVIC_ClearPendingIRQ(PORTA_IRQn);
			
}

// handler UARTa, wpisujemy do niego znaki
// aby pozniej przekazac je na terminal
// dodatkowo obsłużyliśmy działanie backspace tak, by
// można było usuwać tekst, i żeby rzeczywiście usuwał znaki z tablicy
void UART0_IRQHandler()
{
	if(UART0->S1 & UART0_S1_RDRF_MASK)
	{
		var=UART0->D;	
		if (!enter){
			if(var!='\r')
			{			
				while(!(UART0->S1 & UART0_S1_TDRE_MASK)){} ;	
				UART0->D = var;
					
				if (var != 127){	//emulator terminala putty wysyla domyslnie backspace z klawiatury na uarta0 jako ascii delete -> 127 mozna to zmienic w ustawieniach keybord
					rx_buf[rx_buf_pos] = var;	
					rx_buf_pos++;
					if (rx_buf_pos > 49)
						rx_buf_pos--;
				}
				
				else{
					rx_buf_pos--;
					if(rx_buf_pos>49)
						rx_buf_pos++;
					rx_buf[rx_buf_pos] = 0;
				}
			}
			else
				enter = 1;
			
		}
		NVIC_EnableIRQ(UART0_IRQn);
	}
}

// funkcja wysylajaca na uart
void send_uart(char*send) {
  for (int i = 0; send[i] != 0; i++)
  {
    while (!(UART0->S1 & UART0_S1_TDRE_MASK)) {} ; // Czy nadajnik gotowy?
    UART0->D = send[i];
  }
}




void play_letters(char *wynik, int cr) {
	
	
	
	if (S2_press) {
								
							if (cr == 1) {
								for (int i = 0; wynik[i] != 0; i++) {
									if (wynik[i]!=0x20){			
											//sprawdzamy czy dana litera jest samogloska, jesli tak
										  //to ustawiamy parametr "wovel" na 10, aby był on odtworzony 10krotnie
											//jesli nie, to jest ustawiany na 0 i odtwarzamy litere "zwyczajnie"
										if (wynik[i] == 'a'|| wynik[i] == 'e' || wynik[i] == 'o'|| wynik[i] == 'y')
											TPM0_PCM_Play(wynik[i]-97,0);								
										else	
											TPM0_PCM_Play(wynik[i]-97,4);
									}
									DELAY(500)
								}
								S2_press=0;
							}
						
						else if (cr==0) {
							for (int i = 2; wynik[i] != 0; i++) {
									if (wynik[i]!=0x20){			
											//sprawdzamy czy dana litera jest samogloska, jesli tak
										  //to ustawiamy parametr "wovel" na 10, aby był on odtworzony 10krotnie
											//jesli nie, to jest ustawiany na 0 i odtwarzamy litere "zwyczajnie"
										if (wynik[i] == 'a'|| wynik[i] == 'e'|| wynik[i] == 'o'||  wynik[i] == 'y')
											TPM0_PCM_Play(wynik[i]-97,0);								
										else	
											TPM0_PCM_Play(wynik[i]-97,4);
									}
									DELAY(500)
								}
								S2_press=0;
							}
						
}
	
}






int main (void) {
	
	// poczatkowa konfiguracja, inicjalizacja peryferiów.. //
	uint8_t	kal_error;
	char display[106];
	LCD1602_Init();		 // Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);
	LCD1602_SetCursor(2,0);
	LCD1602_Print("skonfiguruj");				// Ekran kontrolny - nie zniknie, jeśli dalsza część programu nie działa
	LCD1602_SetCursor(2,1);
	LCD1602_Print("wcisnij S2");	
	PIT_Init();
	UART0_Init();		// Inicjalizacja portu szeregowego UART0 BR 28800
	TPM0_Init_PCM ();
	Klaw_Init();								// Inicjalizacja klawiatury
	Klaw_S2_4_Int();						// Klawisze S2, S3 zglaszaja przerwanie
	
	/*zmienne do mierzenia napiecia ref*/
	float wyn_temp=0;
	int count_conf = 0;
	float ref = -1;
		
	/* zmienne do wyboru trybu pracy */
	int rcv = 0;
	int snd = 0;		
	
	/*zmienne do wykrywania zmiany poprzedniego stanu*/
	int down = 0;
	int up = 0;
	
	/*zmienne do wyniku detekcji */
	uint16_t znak = 0;
	int miejsce = 0;
	char wynik[50];
	int plc=0;
	
	/*zmienne do kodowania morse'a */
	int wybrano = 0;
	int zly_tekst = 0;
	
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{	
		while(1);									// Klaibracja się nie powiodła
	}
	
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);		// Pierwsze wyzwolenie przetwornika ADC0 w kanale 8 i odblokowanie przerwania
	
	SysTick_Config( 4 * SystemCoreClock/10 ); //liczy do 400ms maks dla 24 bitowego systicka przy clocku 40Mhz(0 - 16 777 215)
	
	sprintf(display,"skonfiguruj\r\nwcisnij S2\r\n");
	send_uart(display);
	
	// początek pętli głównej //
	
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
			
			if (!wybrano){
				sprintf(display,"\r\nwybierz tryb pracy: odbieranie \"rcv\" lub wysylanie \"snd\"\r\nwyswietl tabele morse'a: \"morse\"\r\n");
				send_uart(display);
			}
			
			else{
				sprintf(display,"\r\nnowa konfiguracja gotowa\r\n");
				send_uart(display);
			}
			
			S2_press = 0;
			sek2=0;
			wyn_temp = 0;
			count_conf = 0;
			
		}
		
		
		/*wybieranie trybu pracy przez uarta */
		if (enter && !wybrano){ //wpisano slowo zatwierdzone enterem oraz nie wybrano jeszcze trybu pracy
			
			if (strcmp(rx_buf,"rcv")==0){
				rcv = 1;	
				snd = 0;
				wybrano =1;
				
				sprintf(display,"\r\nwybrano tryb odbierania\r\nwcisnij s3 aby zaczac odbierac lub wpisz \"exit\" by wyjsc:\r\n");
				send_uart(display);
				
			}
			else if (strcmp(rx_buf,"snd")==0){
				snd = 1;
				rcv = 0;
				wybrano =1;
				
				sprintf(display,"\r\nwybrano tryb nadawania morsem\r\nwpisz \"exit\" aby wyjsc lub wpisz kod do nadania:\r\n");
				send_uart(display);
				
			}
			
			else if (strcmp(rx_buf,"morse")==0){
				
				sprintf(display,"\r\nA .-   B -... C -.-. D -..  E .    F ..-.\r\nG --.  H .... I ..   J .--- K -.-  L .-..\r\nM --   N -.");
				send_uart(display);
				
				sprintf(display,"   O ---  P .--. Q --.- R .-. \r\nS ...  T -    U ..-  V ...- W .--  X -..-\r\nY -.-- Z --..\r\n");
				send_uart(display);				
				   
			}			
			
			else{
				rcv = 0;
				snd = 0;
				
				sprintf(display,"\r\nzle -> prosze wpisac rcv, snd lub morse\r\n ");
				send_uart(display);					
			}	
			
			memset(rx_buf, 0, 50);
			rx_buf_pos = 0;			
			enter =0;
		}
		
			
		/* detekcja swiecenia , odbieranie slowa	*/
		
		if(wynik_ok && ref != -1) { //minelo 100ms i skonfigurowano
			if(rcv){ //tryb odbierania
				
				/*wybieranie opcji w ramch trybu*/
				if (enter){
					
					if (strcmp(rx_buf,"exit")==0){
						wybrano =0;
						rcv =0;
						
						memset(wynik, 0, 50);	//reset jakby ktos nie dal s3 wczesniej
						plc =0;
					
						znak=0;
						miejsce=0;					
						
						LCD1602_ClearAll();					// Wyczysc ekran LCD
						LCD1602_SetCursor(2,0);
						sprintf(display,"Uref=%.4fV",ref);
						LCD1602_Print(display);						
						
						sprintf(display,"\r\nwybierz tryb pracy: odbieranie \"rcv\" lub wysylanie \"snd\"\r\nwyswietl tabele morse'a: \"morse\"\r\n");
						send_uart(display);
						
					}
					else{
						
						sprintf(display,"\r\nzle -> prosze wpisac exit lub zaczac nadawac\r\n ");
						send_uart(display);				
					}	
					memset(rx_buf, 0, 50);
					rx_buf_pos = 0;			
					enter =0;
				}				
			
				/*reset/start nadawania*/
				if(S3_press){
					
					LCD1602_ClearAll();					// Wyczysc ekran
					LCD1602_SetCursor(2,0);
					sprintf(display,"Uref=%.4fV",ref);
					LCD1602_Print(display);
					
					sprintf(display,"\r\nzacznij nadawac\r\n");						
					send_uart(display);					
					
					memset(wynik, 0, 50);
					plc =0;
					
					znak=0;
					miejsce=0;	
					
					SysTick->VAL = 0;
					cycles_up = 0;			
					cycles_down = 0;
					
					S3_press = 0;
				}		
				
				if (wyn - ref > 0.20) { //porownanie aktualnej warosci z referencyjna . brak zmiany natezenia swiatla roznica ok. 0 do elsa
																 //zaswiecenie powstaje roznica >0 to nas interesuje , zacienienie roznica <0 wejdzie do elsa
			
				/*wykona sie jesli byl w stanie niskim (nie swiecil) i przeszedl do wysokiego (swiecenie)*/
					if(down) { 
						
						if (cycles_down > 5 && cycles_down < 10) {		//przerwa 2400ms - 3599 ms (6, 9) koniec litery
							if(znak<171)
								wynik[plc++] = alfabet[znak];
							
							znak=0;
							miejsce=0;
						}
									
						else if (cycles_down > 9 && cycles_down < 16) { //przerwa 4400ms - 5999 ms (10, 15)  koniec slowa
							if(znak<171)								
								wynik[plc++] = alfabet[znak];
							wynik[plc++] = 0x20;
							
							znak=0;
							miejsce=0;				
						}

						else if (cycles_down > 15){ //koniec nadawania , nalezy odczekac 6,4 s po ostatnim zakodowanym znaku po czym nadac pojedynczy syganl swietlny
							if(znak<171)
								wynik[plc] = alfabet[znak];
							
							LCD1602_ClearAll();						
							LCD1602_SetCursor(0,0);
							LCD1602_Print("koniec odbierania");
							
							LCD1602_SetCursor(0,1);	
							LCD1602_Print(wynik);
							
							sprintf(display,"\r\nkoniec odbierania\r\nwiadomosc: ");
							send_uart(display);
							
							send_uart(wynik);	
							
							sprintf(display,"\r\nwcisnij s2 aby odtworzyc wiadomosc lub s3 aby wyjsc\r\n");
							send_uart(display);
							
							while(!S3_press){
							
								play_letters(wynik, 1);
							
							}
							sprintf(display,"\r\nwcisnij s3 aby zaczac odbierac lub wpisz \"exit\" by wyjsc:\r\n");
							send_uart(display);
						}
							
						SysTick->VAL = 0;		//start liczenia na poczatku stanu wysokiego
						cycles_up = 0;
						down = 0;
							
					}
					/*wykona sie w kazdym stanie wysokim	*/
					LCD1602_SetCursor(14,0);
					sprintf(display,"%2d",cycles_up);
					LCD1602_Print(display);	
					
					up =1;
						
				}
					
				else {
					
				/*wykona sie jesli byl w stanie wysokim i przeszedl do niskiego*/					
					if(up) {
						
						/*odczyt wskazan licznika , ile czasu trwal stan wysoki */
						if (cycles_up > 0 && cycles_up < 6) {		//swiecilo sie 400ms - 2399ms (1, 5) kropka -> 10 bitowo
							
							znak &=  ~(1<<miejsce);					
							miejsce ++;
							znak |=  1<< miejsce;
							miejsce ++;
		
						}
									
						else if (cycles_up > 5 && cycles_up < 11) { //swiecilo sie 2400 - 4399ms (6, 10)  kreska -> 01 bitowo									
							
							znak |=  1<< miejsce;					
							miejsce ++;
							znak &=  ~(1<<miejsce);		
							miejsce ++;						
										
						}
							
						
						SysTick->VAL = 0;		//start liczenia na poczatku stanu niskiego
						cycles_down = 0;
						
						
						up = 0;
					
					}
					/*wykona sie w kazdym stanie niskim	*/		
					LCD1602_SetCursor(14,1);
					sprintf(display,"%2d",cycles_down);
					LCD1602_Print(display);	
					
					down = 1;		
					
				} 
							

						
				LCD1602_SetCursor(0,1);	
				LCD1602_Print(wynik);
			}
		
			 if(snd){ //tryb nadawania
				
				if (enter){ //wpisano slowo
					
					wynik[plc++] = '\r';	wynik[plc++] = '\n';
					for (int i =0;rx_buf[i]!=0;i++ ){ //sprawdzenie wpisanych liter
						switch (rx_buf[i]){
							
							case '.':	//koduj kropke
								znak &=  ~(1<<miejsce);					
								miejsce ++;
								znak |=  1<< miejsce;
								miejsce ++;
								break;
							
							case '-':	//koduj kreske
								znak |=  1<< miejsce;					
								miejsce ++;
								znak &=  ~(1<<miejsce);		
								miejsce ++;								
							
								break;
							
							case ' ':	//zakoduj litere
								if(znak<171)
									wynik[plc++] = alfabet[znak];							
								
								znak=0;
								miejsce=0;
							
								break;
								
							case '	':	//zakoduj slowo (spacja)
								if(znak<171)
									wynik[plc++] = alfabet[znak];
								wynik[plc++] = 0x20;
								
								znak=0;
								miejsce=0;							
							
								break;
							
							default: //inny znak zle
								zly_tekst = 1;
						}
					}
					if(rx_buf[0]==0)
						zly_tekst=1;
					if(znak<171)
						wynik[plc++] = alfabet[znak];
					
					if (strcmp(rx_buf,"exit")==0){
						wybrano =0;
						snd =0;
						
						LCD1602_ClearAll();					// Wyczysc ekran
						LCD1602_SetCursor(2,0);
						sprintf(display,"Uref=%.4fV",ref);
						LCD1602_Print(display);						
						
						sprintf(display,"\r\nwybierz tryb pracy: odbieranie \"rcv\" lub wysylanie \"snd\"\r\nwyswietl tabele morse'a: \"morse\"\r\n");
						send_uart(display);
						zly_tekst=0;	
					}					
					
					else if (zly_tekst){
						sprintf(display,"\r\nkod moze sie skladac tylko z .(kropka) -(kreska) space(koniec litery) tab(koniec slowa)\r\n");
						send_uart(display);
						zly_tekst=0;
					}
					
					else {
						
						LCD1602_ClearAll();						
						LCD1602_SetCursor(0,0);
						LCD1602_Print("koniec nadawania");
						// poniżej stworzenie nowej tablicy, która będzie
						// wynikiem na LCD, gdyż przez uarta dwa pierwsze
						// miejsca tablicy to Carriage Return oraz New line
						LCD1602_SetCursor(0,1);	
						char *wynik_LCD = wynik;
						wynik_LCD[0] = ' ';
						wynik_LCD[1] = ' ';
						LCD1602_Print(wynik_LCD);	

						//wypisanie wyniku na UART litera po literze
						send_uart(wynik);
						
						sprintf(display,"\r\nwcisnij s2 aby odtworzyc wiadomosc lub s3 aby wyjsc\r\n");
						send_uart(display);

						while(!S3_press){
							// ponizej znajduje sie kod, ktory wygrywa nam kolejne to litery, z nadanego słowa 
							
							play_letters(wynik, 0);
							
						}
						sprintf(display,"\r\nwpisz \"exit\" aby wyjsc lub wpisz kod do nadania:\r\n");
						send_uart(display);
						
					}	

					// resetowanie wszystkich zmiennych
					// po odebraniu wyniku z nadajnika
					// latarka badz tez uart
					memset(wynik, 0, 50);	
					plc =0;
					
					znak=0;
					miejsce=0;						
					
					memset(rx_buf, 0, 50);
					rx_buf_pos = 0;			
					enter =0;
					DELAY(1000);
					S3_press=0;
				}
				
			}
			wynik_ok = 0;
		}	
		
		
	}	
	
}