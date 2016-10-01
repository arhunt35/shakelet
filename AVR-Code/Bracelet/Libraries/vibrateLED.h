/* Functions to initialize, send, receive over USART

   initUSART requires BAUD to be defined in order to calculate
     the bit-rate multiplier.
 */


extern void initVibrate(void);

/* Sets the register bits to initialise vibration motor.*/

extern void initLEDs(void);

/* Sets register bits for LEDs*/

extern void flipVibrate(int);

/* Flips the vibration on for int ms.*/

extern void flipLED(int);

/* Flips the LED. 0=R, 1=G, 2=B*/

extern void LEDVibrate(int,int);

/* Turns on both LED and vibrator for ms*/

extern void vibrate_fuzz(void);

/* Vibrate pattern*/

extern void vibrate_sos(void);

/* Vibrate pattern*/

extern void vibrate_solid(void);

/* Vibrate pattern*/

extern void vibrate_random(void);

/* Vibrate pattern*/