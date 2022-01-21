/******************************************************************************
 * This file is a part of the Sysytem Microprocessor Tutorial (C).            *
 ******************************************************************************/

/**
 * @file tpm_pcm.c
 * @author Koryciak
 * @date Oct 2021
 * @brief File containing definitions for TPM.
 * @ver 0.1
 */

#include "tpm_pcm.h"
#include "music_samples.h"

/******************************************************************************\
* Private definitions
\******************************************************************************/
#define UPSAMPLING 10
/******************************************************************************\
* Private prototypes
\******************************************************************************/
void TPM0_IRQHandler(void);
/******************************************************************************\
* Private memory declarations
\******************************************************************************/
static uint8_t tpm0Enabled = 0;

static uint16_t upSampleCNT = 0;
static uint16_t sampleCNT = 0;
static uint32_t  playFlag = 0;
static uint8_t vowelFlag =10;

void TPM0_Init_PCM(void) {
		
  SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;		
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);    // CLK ~ 41,9MHz (CLOCK_SETUP=0)

	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK; 
	PORTB->PCR[7] = PORT_PCR_MUX(2);			// PCM output -> PTB7, TPM0 channel 2
	
	TPM0->SC |= TPM_SC_PS(2);  					  // 41,9MHz / 4 ~ 10,48MHz
	TPM0->SC |= TPM_SC_CMOD(1);					  // internal input clock source

	TPM0->MOD = 255; 										  // 8bit PCM
																				// 10,48MHz / 256 ~ 40,96kHz
	
// "Edge-aligned PWM true pulses" mode -> PCM output
	TPM0->SC &= ~TPM_SC_CPWMS_MASK; 		
	TPM0->CONTROLS[2].CnSC |= (TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK); 
	TPM0->CONTROLS[2].CnV = 0; 
	
	
// "Output compare" -> for intterrupt
	TPM0->SC &= ~TPM_SC_CPWMS_MASK;
	TPM0->CONTROLS[0].CnSC |= (TPM_CnSC_MSA_MASK | TPM_CnSC_ELSA_MASK);
	TPM0->CONTROLS[0].CnV = 0;
  
	TPM0->CONTROLS[0].CnSC |= TPM_CnSC_CHIE_MASK; // Enable interrupt
	
	NVIC_SetPriority(TPM0_IRQn, 1);  /* TPM0 interrupt priority level  */

	NVIC_ClearPendingIRQ(TPM0_IRQn); 
	NVIC_EnableIRQ(TPM0_IRQn);	/* Enable Interrupts */
	
	tpm0Enabled = 1;  /* set local flag */
}

// zmienna letter_play jest do wyboru sciezki dzwiekowej 
// zmienna sample zawiera ilosc probek litery 
// samogloske zapetlamy 10krotnie, tworzac jedynie ok 100 próbek
// na litere tak, aby zaoszczedzic pamiec

static const uint8_t* letter_play;
int sample;

void TPM0_PCM_Play(int letter, int vowel) {
	
	vowelFlag = vowel;
	letter_play = all_letter[letter];
	sample = samples[letter];
	
	sampleCNT = 0;   /*zacznij liczyc od poczatku, gdy odtwarzamy nowa litere  */
	playFlag = 1;
	
}

void TPM0_IRQHandler(void) {
	
	if (playFlag) {
		if (upSampleCNT == 0)TPM0->CONTROLS[2].CnV = letter_play[sampleCNT++]; // w tej linijce wgrywamy kolejne probki
		if (sampleCNT > sample) {
			// tutaj, jesli 
			if (vowelFlag<4){
				sampleCNT=0;
				vowelFlag++;
			}
			else {
			playFlag = 0;         // stop if at the end
			TPM0->CONTROLS[2].CnV = 0;
			}
		}
		// 40,96kHz / 10 ~ 4,1kHz ~ WAVE_RATE
		if (++upSampleCNT > (UPSAMPLING-1)) upSampleCNT = 0;
	}
	
	TPM0->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK; 
}
