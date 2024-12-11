//
// Cris, March 2013
// this is a simple example, which drives a stepper motor in one
// direction or the other based on the control from the joystick
// on the MCB1700 board;
//
// TODO (assignments to students):
// 1. use Timer 0 interrupt to generate delays (replace the use of 
//    delay_dirty function)
// 2. drive also four of the 8 LEDs on the board to show the output
//    signals that that drive the stepper (basically replicate what
//    the LEDs on the stepper's little board do)
// 3. use the ADC to control the speed of the stepper (which should
//    be proportional to the value of the potentiometer on the board)
// Note: all the elements to implement each of the above assignments
// have been studied in the previous labs!
//

#include "LPC17xx.h"
#include "joystick.h"
#include "GLCD.h"
#define __FI        1                   /* Font index 16x24                  */
uint8_t lookup_table[4][4] = {
    {1,0,0,0},
    {0,1,0,0},
    {0,0,1,0},
    {0,0,0,1} };


void set_outputs( int index)
{
    // drive the four main outputs that control the stepper;
    // use the values from the look up table;
    
    if ( lookup_table[index][0] == 0) {
        LPC_GPIO0->FIOCLR |= 1 << 0;
    } else {
        LPC_GPIO0->FIOSET |= 1 << 0;        
    }
    if ( lookup_table[index][1] == 0) {
        LPC_GPIO0->FIOCLR |= 1 << 1;
    } else {
        LPC_GPIO0->FIOSET |= 1 << 1;        
    }    
    if ( lookup_table[index][2] == 0) {
        LPC_GPIO0->FIOCLR |= 1 << 2;
    } else {
        LPC_GPIO0->FIOSET |= 1 << 2;        
    }
    if ( lookup_table[index][3] == 0) {
        LPC_GPIO0->FIOCLR |= 1 << 3;
    } else {
        LPC_GPIO0->FIOSET |= 1 << 3;        
    }
}


// we'll use delay_dirty() as a software delay function; it should produce 
// about a second when del=(1 << 24) or so of delay depending on CCLK; 
volatile uint32_t temp;
void delay_dirty( uint32_t del)
{
	uint32_t i;
	for ( i=0; i<del; i++) { temp = i; }
}
void GLCD_for_fullstepper(uint32_t index){
	switch(index){
								case (0):
								  GLCD_SetBackColor(White);
									GLCD_SetTextColor(Purple);
									GLCD_DisplayChar(5, 7+0, __FI, 0x80+1);  
									GLCD_DisplayChar(5, 7+1, __FI, 0x80+0);
									GLCD_DisplayChar(5, 7+2, __FI, 0x80+0);  
									GLCD_DisplayChar(5, 7+3, __FI, 0x80+0);
								break;
								case (1):
								  GLCD_SetBackColor(White);
									GLCD_SetTextColor(Purple);
									GLCD_DisplayChar(5, 7+0, __FI, 0x80+0);  
									GLCD_DisplayChar(5, 7+1, __FI, 0x80+1);
									GLCD_DisplayChar(5, 7+2, __FI, 0x80+0);  
									GLCD_DisplayChar(5, 7+3, __FI, 0x80+0);
								break;
								case (2):
								  GLCD_SetBackColor(White);
									GLCD_SetTextColor(Purple);
									GLCD_DisplayChar(5, 7+0, __FI, 0x80+0);  
									GLCD_DisplayChar(5, 7+1, __FI, 0x80+0);
									GLCD_DisplayChar(5, 7+2, __FI, 0x80+1);  
									GLCD_DisplayChar(5, 7+3, __FI, 0x80+0);
								break;
								case (3):
								  GLCD_SetBackColor(White);
									GLCD_SetTextColor(Purple);
									GLCD_DisplayChar(5, 7+0, __FI, 0x80+0);  
									GLCD_DisplayChar(5, 7+1, __FI, 0x80+0);
									GLCD_DisplayChar(5, 7+2, __FI, 0x80+0);  
									GLCD_DisplayChar(5, 7+3, __FI, 0x80+1);
								break;
								default:
								break;
							}
						}

int main( void) 
{
    uint32_t joy_keys;
    uint32_t MY_DELAY = (1 << 14);
    uint32_t i;
    uint32_t k;
    // (1) Power up GPIO   
    LPC_SC->PCONP |= ( 1 << 15 ); 
    
    LPC_GPIO4->FIODIR   |=  (1UL<< 28); // Pin P2.28 is output (GLCD BAcklight)
    LPC_GPIO4->FIOPIN   &= ~(1UL<< 28); // Turn backlight off 
    
    // (2) use P0.0..3 as the four signals to drive the stepper motor;
    LPC_GPIO0->FIODIR |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
    LPC_GPIO0->FIOCLR |= 1 << 0;
    LPC_GPIO0->FIOCLR |= 1 << 1;
    LPC_GPIO0->FIOCLR |= 1 << 2;
    LPC_GPIO0->FIOCLR |= 1 << 3;    
		GLCD_Init();                              /* Initialize the GLCD           */
		GLCD_Clear(White);                        /* Clear the GLCD                */
		GLCD_SetBackColor(Blue);
    GLCD_SetTextColor(White);
    GLCD_DisplayString(0, 0, __FI, "  Lab 5 Assignment  ");
    GLCD_DisplayString(1, 0, __FI, "Full stepper + GLCD ");
    GLCD_DisplayString(2, 0, __FI, "  Embedded lab IU   ");
		GLCD_SetBackColor(Cyan);
    GLCD_SetTextColor(Black);
    GLCD_DisplayString(4, 7, __FI, "A");
		GLCD_SetBackColor(Green);
    GLCD_SetTextColor(Black);
    GLCD_DisplayString(4, 8, __FI, "B");
		GLCD_SetBackColor(Yellow);
    GLCD_SetTextColor(Black);
    GLCD_DisplayString(4, 9, __FI, "C");
		GLCD_SetBackColor(Red);
    GLCD_SetTextColor(Black);
    GLCD_DisplayString(4, 10, __FI, "D");
    // (3) infinite loop;
    while (1) 
    {
        // (a) read joystick keys;
        joy_keys = JOYSTICK_GetKeys();
        
        // (b) drive the stepper motor;
        // TODO: this way of driving the stepper is what I call a 
        // "hack"; it's not the best one as it requires: 1) to tune
        // MY_DELAY until the frequency of the four signals driving 
        // the stepper is in the range of 100kHz (from the specs of the
        // stepper motor, given to us by manufacturer) and 2) the actual
        // operation has a subtle issue: you need to go thru all 8 steps
        // before direction can be changed; think about it! this can be
        // addressed by working with say teh systick interrupt (see blinky2
        // example from lab1) whcih would require some custom code in
        // void SysTick_Handler(void), where we would read the joystick
        // keys and set a global variable with the direction; this variable
        // then would be utilized here to increment or decrement "i";
        if ( joy_keys & JOYSTICK_LEFT) {
            for ( i = 4; i > 0; i-- ) {
                set_outputs( i-1);
								GLCD_for_fullstepper(i-1);
                delay_dirty( MY_DELAY);
            }
        } else if ( joy_keys & JOYSTICK_RIGHT) {
            for ( i = 0; i < 4; i++ ) {
                set_outputs( i);
								GLCD_for_fullstepper(i);
                delay_dirty( MY_DELAY);
            }           
        }
    }
    
    return 0;  
}
