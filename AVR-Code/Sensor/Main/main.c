// ------- Preamble -------- //
#include <avr/io.h>                  	// Defines pins, ports, etc
#include <util/delay.h>            		// Functions to waste time
#include <stdlib.h>
#include "pinDefines.h"
#include "ADC.h"
#include "nrf24l01.h"
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "USART.h"

volatile uint8_t sensorID = 2;

volatile uint16_t potValue = 1;
volatile uint16_t piezoValue = 1;
volatile uint16_t baseValue = 1023;

volatile uint16_t lowThresh = 127;
volatile uint16_t highThresh = 127;
volatile uint16_t middleValue = 512;

volatile uint8_t W_buffer[3];

volatile uint32_t combinedADC;
volatile uint16_t lowADC;


void sendSignal(void){
	
	W_buffer[0]=sensorID;
	W_buffer[1]=piezoValue;
	W_buffer[2]=potValue;
	
	transmit_payload(W_buffer);
	reset();
	flashLED();
	//USARTPrint();
	
}

void flashLED(){

	LED_PORT |= (1 << LED_RED)|(1 << LED_GREEN);
	LED_PORT2 |= (1 << LED_BLUE);
	LED_PORT &= ~(1 << LED_RED);
	LED_PORT &= ~(1 << LED_GREEN); 	// Clear all LEDs by setting pin low
	LED_PORT2 &= ~(1 << LED_BLUE); 	// Clear LED by setting pin low

}

void USARTPrint(){
	//USART code for debugging purposes//
	printString("Piezo Value: ");
	printWord(piezoValue);
	printString(" Pot Value: ");
	printWord(potValue);
	printString(" Middle Value: ");
	printWord(middleValue);
	printString("\r\n");
}
	
ISR(ADC_vect){
	//Convert ADCH and ADCL to single integer with range 0 to 1023
	lowADC = ADCL;
	combinedADC = (ADCH<<8)|lowADC;
	
	//Check which ADC conversion just completed and assign ADC value to correct integer
	if(ADMUX & (1<<MUX0)){
		piezoValue = combinedADC;
		middleValue = ((middleValue * 7) + (piezoValue * 3))/10;
	}else{
		potValue = combinedADC*200/baseValue; //Convert to a figure between 0 and 50 to add as padding
		lowThresh = (middleValue - (combinedADC*200)/baseValue);
		highThresh = (middleValue + (combinedADC*200)/baseValue);
	}
	
	//Flip Mux0 to change between ADC0 and ADC1 channel (to swap between piezo and potentiometer
	ADMUX ^= (1<<MUX0);
	
	
	if(piezoValue<lowThresh||piezoValue>highThresh){
		sendSignal();
		_delay_ms(50);		//Don't send signal too often - causes issues with receiver.
	}
	
}


int main(void) 
{
	
	initSPI();
	initNRF24L01();
	//initUSART();
	
	//Init LEDs and flash once to show script running
	LED_DDR|=(1 << LED_RED)|(1 << LED_GREEN);		// set LED pins for output
	LED_DDR2|=(1 << LED_BLUE);		// set blue LED pin for output
	flashLED();  				//Flash to show initiated
	
	initADC();
	ADCSRA |= (1<<ADSC);		// Start conversion - didn't do earlier as not all bits set

	//Reduce power
	power_timer0_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_twi_disable();
		
	//Sleep modes
	set_sleep_mode(SLEEP_MODE_ADC);
	sleep_enable();
	sleep_bod_disable();
	sleep_cpu();
	sleep_mode();
	
	//Power down nrf24
	
	// Mainloop - use asm to stop empty infinite loop being optimised out.
  while(1)
  {
	asm("");
  }

  return 0;
}

	