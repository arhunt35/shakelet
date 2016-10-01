#include <avr/io.h>
#include "ADC.h"
#include "pinDefines.h"
#include <util/delay.h>                     
#include <stdlib.h>
#include <avr/interrupt.h>

void initADC(void) {

	//Set pin for input and enable pull up resistor
	PIEZO_DDR &= ~(1 << PIEZO);
	
	//See p261 of datasheet - ADMUX sets which pin, voltage ref and left shift
	ADMUX =
            
			(0 << REFS1) |     	// Sets ref. voltage to VCC, bit 1 (use 01 for VCC)
            (1 << REFS0) |     	// Sets ref. voltage to VCC, bit 0 
            (0 << ADLAR) |     	// Left or right shift (1 = left shift - Most Significant Bit in ADCH, LSB in ADCL)
			(0 << MUX3)  |     	// Pin select bit 3 (0000= ADC0 - PC0, 0001= ADC1 - PC1)
            (0 << MUX2)  |     	// Pin select bit 2
            (0 << MUX1)  |     	// Pin select bit 1
            (0 << MUX0);       	// Pin select bit 0

	//See p263 of datasheet - ADCSRA = ADC Control and Status Register A
	ADCSRA = 
            (1 << ADEN)  |     	// Enable ADC (1 = enabled)
            (0 << ADSC)  |     	// Start conversion (1 = start)
			(1 << ADATE) |     	// ADC trigger enable (for using external trigger or free-running set to 1)
			(0 << ADIF)  |     	// ADC interrupt flag - auto set by hardware.  Don't change
			(1 << ADIE)  |     	// Enables interrupt mode (1 = enabled) 
			(1 << ADPS2) |     	// ADC Prescaler bit 2 (110 = 64 Prescaler.  Reduces ADC frequencey to 125kHz when chip at 8MHz)
			(1 << ADPS1) |     	// ADC Prescaler bit 1
			(0 << ADPS0);		// ADC Prescaler bit 0
			
	//See p265 of datasheet - ADCSRB = ADC Control and Status Register B.  Only 6 usable bits here
	ADCSRB =  
            (0 << ACME)  |		// ?
			(0 << ADTS2) |		// Auto trigger source bit 2 (000 will be free running mode)
			(0 << ADTS1) |		// Auto trigger source bit 1
            (0 << ADTS0);		// Auto trigger source bit 0
	
	//See p138 - Disable digital input on pin to reduce power consumption (write 1 when using pin only for ADC i.e. digital not needed)
	DIDR0 = 
			(0 << ADC0D) |		// Disable digital in on ADC0 (PC0)
            (0 << ADC1D) |		// Disable digital in on ADC1 (PC1)
			(0 << ADC2D) |		// Disable digital in on ADC2 (PC2)
			(0 << ADC3D) | 		// Disable digital in on ADC3 (PC3)
			(0 << ADC4D) |		// Disable digital in on ADC4 (PC4)
			(0 << ADC5D);		// Disable digital in on ADC5 (PC5)
	
	sei();						// Enable interrupts
	ADCSRA |= (1<<ADSC);		// Start conversion - didn't do earlier as not all bits set

}