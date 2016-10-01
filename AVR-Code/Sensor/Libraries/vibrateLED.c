#include <avr/io.h>
#include "vibrateLED.h"
#include "pinDefines.h"
#include <util/delay.h>                     
#include <stdlib.h>


// Initialisation codes

void initVibrate(void) {                                
  VIBRATE_DDR|=(1 << VIBRATE_DISK);  								// Set Vibration pin for output
}

void initLEDs(void){
	LED_DDR|=(1 << LED_RED)|(1 << LED_GREEN)|(1 << LED_BLUE);		// set LED pins for output
	LED_PORT |= (1 << LED_RED)|(1 << LED_GREEN)|(1 << LED_BLUE); 	// Clear all LEDs by setting pins high (common anode therefore sinking / set high to clear)
}


// Flip codes

void flipVibrate(int duration){
	VIBRATE_PORT ^= (1<< VIBRATE_DISK);
	_delay_ms(duration);

}

void flipLED(int colour){	
	if(colour==0){
		LED_PORT ^= (1 << LED_RED);
		}
	else if(colour==1){
		LED_PORT ^= (1 << LED_GREEN);
		}
	else{
		LED_PORT ^= (1 << LED_BLUE);
	}
	
}

void LEDVibrate(int duration, int colour){
	if(colour==0){
		LED_PORT &= ~(1 << LED_RED);
		VIBRATE_PORT |= (1<< VIBRATE_DISK);
		}
	else if(colour==1){
		LED_PORT &= ~(1 << LED_GREEN);
		VIBRATE_PORT |= (1<< VIBRATE_DISK);
		}
	else{
		LED_PORT &= ~(1 << LED_BLUE);
		VIBRATE_PORT |= (1<< VIBRATE_DISK);
	}
	
	_delay_ms(duration);
	VIBRATE_PORT &= ~ (1<< VIBRATE_DISK);
	LED_PORT |= (1 << LED_RED)|(1 << LED_GREEN)|(1 << LED_BLUE);
}

// Vibrate patterns
void vibrate_fuzz(){
	int x, y;
	
	for(x=0; x<1; x++){
		for(y=1; y<125; y=y+3){
			LEDVibrate(y,1);
			_delay_ms(y);
		}
	}
}

void vibrate_sos(){
	int x,dot,dash,pause;
	dot=150;
	dash=275;
	pause=175;
	for(x=0; x<3; x++){
			LEDVibrate(dot,0);
			_delay_ms(pause);
	}
	_delay_ms(pause);
	for(x=0; x<3; x++){
			LEDVibrate(dash,0);
			_delay_ms(pause);
	}
	_delay_ms(pause);
	for(x=0; x<3; x++){
			LEDVibrate(dot,0);
			_delay_ms(pause);
	}
	_delay_ms(275);
}

void vibrate_solid(){
	int x;
	for(x=0; x<9; x++){
			LEDVibrate(300,3);
			_delay_ms(300);
	}
	_delay_ms(275);
}

void vibrate_random(){
	int random1,x;
	int pulse = 15;
	random1 = rand() % 3;
	for(x=0;x<175;x++){
		LEDVibrate(pulse,random1);
		_delay_ms(pulse);
		random1 = rand() % 3;
	}
	
}