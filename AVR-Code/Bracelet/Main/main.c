// ------- Preamble -------- //
#include <avr/io.h>                  	// Defines pins, ports, etc
#include <util/delay.h>            		// Functions to waste time
#include <stdlib.h>
#include "pinDefines.h"
#include "vibrateLED.h"
#include "nrf24l01.h"
#include "USART.h"
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

volatile uint8_t *data; //variable for holding received data

void printUSART(){
	/* USART readout for debugging purposes*/
	printString("Sensor ID: ");
	printByte(data[0]);
	printString(" Piezo Value: ");
	printByte(data[1]);
	printString(" Pot Value: ");
	printByte(data[2]);
	printString("\r\n");
}

void powerSaving(){
	power_adc_disable();
	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();
}

ISR(INT0_vect){ //Interrupt routine for NRF24
	
	EIMSK &= ~(1<<INT0);		//Turn off INT0
	data=WriteToNrf(R, R_RX_PAYLOAD, data, 5);
	reset();

	//Work out which sensor is triggered and use correct vibrate pattern
	switch(data[0]){
		case 0:
			vibrate_sos();
			break;
		case 1:
			vibrate_solid();
			break;
		case 2:
			vibrate_random();
			break;
		default:
			vibrate_fuzz();
			break;
	}
	EIMSK |= (1<<INT0);		//Turn on INT0
}


int main(void) {

	// Initialisations
		initLEDs();						// Initialise LED pins
		initVibrate(); 					// Initialise vibration pin
		initSPI();
		initNRF24L01();
		initUSART();
		
		SETBIT(RF_PORT, 1);	//CE HIGH = NRF24 is listening
		
		LEDStart(); //Flash LED to indicate that chip has booted up
		
		INT0_interrupt_init();
		
		while(getReg(CONFIG)!=0x1F){
		//Show red light when nsf24L01 us not ready to receive
			LED_PORT |= (1 << LED_GREEN);
			LED_PORT &= ~(1 << LED_RED);
			LED_PORT |= (1 << LED_BLUE);
			initNRF24L01();
			LED_PORT |= (1 << LED_RED);
		}
		
	// Mainloop - use asm to stop empty infinite loop being optimised out.
	while (1) {
		asm("");
	}
  return (0);            
}