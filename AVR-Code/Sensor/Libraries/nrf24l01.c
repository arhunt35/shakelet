// FOR ATMEGA168

#include <avr/io.h>
#include <stdio.h>
#include "nrf24l01.h"
#include "pinDefines.h"
#include <util/delay.h>                     
#include <stdlib.h>
#include <avr/interrupt.h>

#define F_CPU 800000
#define dataLen 3

// Initialisation codes

void initSPI(void){
	//Set SCK (PB5), MOSI (PB3) , CSN (PB2) & CE (PB1) as outport 
	RF_DDR |= (1<<RF_SCK) | (1<<RF_MOSI) | (1<<RF_CSN) |(1<<RF_CE);
	
	/* Enable SPI, Master, set clock rate */
	SPCR |= (1<<SPE)|(1<<MSTR);//|(1<<SPR0);
	
	SETBIT(PORTB, 2);	//CSN IR_High to start with
	CLEARBIT(PORTB, 1);	//CE low to start with
}


//FUNCTION TAKEN DIRECTLY FROM DATA SHEET - PAGE 168
char WriteByteSPI(unsigned char cData){
	//Load byte to Data register
	SPDR = cData;	

	//Wait for transmission to complete
	while(!(SPSR & (1<<SPIF)));

	//Return data from register
	return SPDR;
}


//Function tested and working
uint8_t getReg(uint8_t reg){	
	_delay_us(10);
	CLEARBIT(RF_PORT, 2);			//Set CSN low - makes NRF24 listen for data
	_delay_us(10);					//Requires delay of 10us before will accept data - only one byte!!
	WriteByteSPI(R_REGISTER + reg);	//Sets nRF to reading mode for register "reg"
	_delay_us(10);
	reg = WriteByteSPI(NOP);		//Send NOP (dummy byte) once to receive back the first byte in the register - NOP defined in .h
	_delay_us(10);
	SETBIT(RF_PORT, 2);				//CSN - set high - returns to doing nothing
	return reg;						//Return the registry
}



//Slightly complex function to read or write to or from nrf24
uint8_t *WriteToNrf(uint8_t ReadWrite, uint8_t reg, uint8_t *val, uint8_t antVal){
	//cli();	//disable global interrupt
	
	//check whether read or write ("W or R") passed, "reg" - register to target, *val - array with the package to read / write, antval - number of integers in the package
	//If write - all the write bit to passed reg
	if (ReadWrite == W)	//W = write, R = Read
	{
		reg = W_REGISTER + reg;	//add the "write" bit to the reg
	}

	//Create an array to be returned at the end
	static uint8_t ret[32];	
	
	_delay_us(10);		//Give space since last command
	CLEARBIT(PORTB, 2);	//CSN low - nRF starts to listen for a command
	_delay_us(10);		
	WriteByteSPI(reg);	//set nRF to write or read mode of "reg"
	_delay_us(10); 		
	
	int i;
	for(i=0; i<antVal; i++) //loop through number of integers in array
	{
		if (ReadWrite == R && reg != W_TX_PAYLOAD) //When writing to W_TX_Payload you cannot add the W
		{
			ret[i]=WriteByteSPI(NOP);	//Send dummy bytes to read back data
			_delay_us(10);			
		}
		else 
		{
			WriteByteSPI(val[i]);	//Send the commands to the nRF one at a time
			_delay_us(10);
		}		
	}
	SETBIT(PORTB, 2);	//Set CSN high - nRF goes back to idling
	
	//sei(); //enable global interrupt
	
	return ret;	//return the array
}


//Code to initialise all the right bits on the NRF24L01
void initNRF24L01(void){
	_delay_ms(100);	//allow radio to reach power up if shut down
	
	uint8_t val[5];	//Establish array of integers to send

	//EN_AA - (Enable auto-acknowledgements)
	val[0]=0x01;	//Set value
	WriteToNrf(W, EN_AA, val, 1);	//W=write mode, EN_AA = Auto Acknowledgement register, val 0x01 = 1 (positive), 1 = number of integers
	
	//SETUP_RETR (number of retries for auto acknowledge)
	val[0]=0x2F;	//0b0010 00011 "2" sets it up to 750uS delay between every retry (at least 500us at 250kbps and if payload >5bytes in 1Mbps, and if payload >15byte in 2Mbps) "F" is number of retries (1-15, now 15)
	WriteToNrf(W, SETUP_RETR, val, 1);
	
	//Choose data pipe (pipe 0 used here)
	val[0]=0x01;
	WriteToNrf(W, EN_RXADDR, val, 1);

	//RF_Adress width setup (how many bytes is the receiver address (1-5)
	val[0]=0x03;
	WriteToNrf(W, SETUP_AW, val, 1); //0b0000 00011 = 5 bytes

	//RF channel setup - Choose frequency (2,400 to 2,527 GHz) in 1 MHz steps
	val[0]=0x01;
	WriteToNrf(W, RF_CH, val, 1); //RF channel registry 0b0000 0001 = 2,401GHz

	//RF setup	- Choose power mode and data speed
	val[0]=0x26;
	WriteToNrf(W, RF_SETUP, val, 1); //00000111 bit 3="0" 1Mbps = longer range, bit 2-1 power mode. 0x26 = long range.  0x07 = short range.

	//RX RF_Adress setup 5 byte - Set Receiver address
	int i;
	for(i=0; i<5; i++)	
	{
		val[i]=0x12;	//Gets a long secure address
	}
	WriteToNrf(W, RX_ADDR_P0, val, 5); //Assign address to pipe 0
	
	//TX RF_Adress setup 5 byte -  Setup transmitter address
	for(i=0; i<5; i++)	
	{
		val[i]=0x12;	//Must set same on receiver chip
	}
	WriteToNrf(W, TX_ADDR, val, 5);

	// payload width setup - how many bytes per transmission
	val[0]=dataLen;		//Using 5 bytes per package
	WriteToNrf(W, RX_PW_P0, val, 1);
	
	//CONFIG reg setup - Boot up NRF and choose if transmitting or receiving
	val[0]=0x1E;  //Use 0x1F for Receiver, 0x1E for Transmitter.  0b0000 1111 config registry	bit "1":1=power up,  bit "0":0=transmitter (bit "0":1=Reciver) (bit "4":1=>mask_Max_RT,dvs IRQ-vektorn reagerar inte om sändningen misslyckades. 
	WriteToNrf(W, CONFIG, val, 1);

//device need 1.5ms to reach standby mode (CE=Low)
	_delay_ms(100);	

	//sei();	
}

void transmit_payload(uint8_t * W_buff){
	WriteToNrf(R, FLUSH_TX, W_buff, 0); //Send 0xE1 to flush out old data
	WriteToNrf(R, W_TX_PAYLOAD, W_buff, dataLen);	//Send the data in W_buff to the nrf Payload register
	
	//sei();	//enable global interrupt- if used

	_delay_ms(10);		//Need a 10ms delay
	SETBIT(RF_PORT, 1);	//CE high = transmit the data
	_delay_us(20);		//Delay of at least 10us
	CLEARBIT(RF_PORT, 1);	//CE low = stop transmitting
	_delay_ms(10);		//Delay before any further functions

	//cli();	//Disable global interrupt if used

}

void receive_payload(void){
	//sei();		//Enable global interrupt
	
	SETBIT(RF_PORT, 1);	//CE HIGH = listening
	_delay_ms(1000);	//Listens for 1s
	CLEARBIT(RF_PORT, 1); //CE LOW = stop listening
	
	//cli();	//Disable global interrupt
}

void reset(void)
{
	_delay_us(10);
	CLEARBIT(PORTB, 2);	//CSN low
	_delay_us(10);
	WriteByteSPI(W_REGISTER + STATUS);	//
	_delay_us(10);
	WriteByteSPI(0b01110000);	//Reset all IRQs
	_delay_us(10);
	SETBIT(PORTB, 2);	//CSN IR_High
}

void INT0_interrupt_init(void){
	DDRD &= ~(1<DDD2);			// Set PD2 pin as input
	PORTD |= (1 << PORTD2);    // turn On the Pull-up
	
	EICRA |= (1<<ISC01);	    // set INT0 to trigger on ANY logic change
	
	
	EIMSK |= (1<<INT0);		//Turn on INT0
	
	//sei();						//Turn on interrupts

}