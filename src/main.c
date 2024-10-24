/* ------------------------------------------
       ECS642/ECS714 Lab2 KL28Z version

   The aim of this lab exercise is to understand how to react
   to button presses.
	 
	 CHANGES IN CODE! OGULCAN GURELLI EC21698
	 
	 This code helps to change the base LED system for LAB2. I created a new method to invert the 
	 LEDS when the appropriate button is clicked, with case statements to change the direction and setting of LEDS, such as clockwise to anticlockwise, inverse to non-inverse.
	 
	 I've created new variables to match the directions that are desired in this lab, and an array of all 5 LEDS, so the current state can
	 traverse among the LEDS.
	 
	 I've set the default direction to clockwise.
  -------------------------------------------- */
  
#include <stdbool.h>
#include <stdint.h>
#include <MKL28Z7.h>
#include "..\inc\SysTick.h"
#include "..\inc\button.h"
#include "..\inc\led.h"



/* --------------------------------------
     Documentation of the Given Code
     ==============================

     Behaviour
     ---------     
     Two buttons on the shield are used:
        * Button B1 is polled. Each press toggles the state of the LED1 
        * Button B5 has an interrupt handler. Each press toggles state of LED5
     
     Files
     -----
     The following file may need to be changed to complete the exercise
     
     main.c  Contains the main loop with 3 tasks and an interrupt handler
     
     The following files should not be changed:
       mask.h   Definition of the MASK macro
       button.h  Definitions and API for shield buttons
       button.c  Code to configure buttons and detect button position
       led.h   Definitions and API for shield LEDs
       led.c   Code to configure and switch shield LEDs
       SysTick.h  Definitions and API for SysTick timer
       SySTick.c  Code for SySTick timer
 -------------------------------------- */

/*----------------------------------------------------------------------------
  Variables for communication
*----------------------------------------------------------------------------*/
bool pressedB1_ev ;  // set by task1 (polling) and cleared by task 2
volatile bool pressedB5_ev ;  // Set by interrupt handler
                              // Cleared by the task3
                             
/*----------------------------------------------------------------------------
  task1pollB1
  
  This task polls button B1
*----------------------------------------------------------------------------*/
int b1State ;        // Current state - corresponds to position
int b1BounceCount ;

void initTask1() {
    b1State = BOPEN ;
    pressedB1_ev = false ; 
    b1BounceCount = 0 ;
}

void task1pollB1() {
    if (b1BounceCount > 0) b1BounceCount -- ;
    switch (b1State) {
        case BOPEN:
            if (isPressed(B1)) {
                b1State = BCLOSED ;
                pressedB1_ev = true ; 
            }
          break ;

        case BCLOSED:
            if (!isPressed(B1)) {
                b1State = BBOUNCE ;
                b1BounceCount = BOUNCEDELAY ;
            }
            break ;

        case BBOUNCE:
            if (isPressed(B1)) {
                b1State = BCLOSED ;
            }
            else if (b1BounceCount == 0) {
                b1State = BOPEN ;
            }
            break ;
    }                
}


/*----------------------------------------------------------------------------
  task2LED1
  
  This task toggles the state of LED1 when the 'B1 pressed' event occurs
*----------------------------------------------------------------------------*/
#define CLOCKWISE (0) /* 4 DIRECTION SECTIONS SYSTEMS*/
#define ANTICLOCKWISE (1)
#define CLOCKWISE_INVERSE (2)
#define ANTICLOCKWISE_INVERSE (3)
#define delayPeriod (100)
int direction = CLOCKWISE;
int current = -1; //current led
int delayCount =delayPeriod; //DELAY COUNT DECREMENTS WITH EACH CYCLE
const int leds[] = {LED1,LED2,LED3,LED4,LED5}; /* All 5 LEDS*/



void invertLEDs(int current){
	switch (direction){
		case CLOCKWISE: //FOR CLOCKWISE AND ANTICLOCKWISE, THE FUNCTION TURNS ON ALL LEDS EXCEPT THE CURRENT ONE
  	case ANTICLOCKWISE: 
			for (int i=0; i<5;i++){
				if (i!=current){
					ledOnOff (leds[i],LED_ON); // TURNS ON ALL LEDS EXCEPT THE CURRENT ONE
				}
				else{
					ledOnOff(leds[i],LED_OFF); // TURNS OFF ALL THE CURRENT LEDS
				}
			}
			break; 

		case CLOCKWISE_INVERSE: //FOR CLOCKWISE INVERSE AND ANTICLOCKWISE INVERSE, THE FUNCTION TURNS OFF ALL LEDS EXCEPT THE CURRENT ONE
		case ANTICLOCKWISE_INVERSE:
			for (int i=0; i<5;i++){
				if (i!=current){
					ledOnOff (leds[i],LED_OFF); //TURNS OFF ALL LEDS EXCEPT THE CURRENT ONE
				}
				else{
					ledOnOff(leds[i],LED_ON);
				}
			}
			break;
	}
}


void initTask2() {
    for (int i=0; i<5;i++){
			ledOnOff (leds[i],LED_OFF);
		}
}


void task2LED() {
    // ONE CASE FOR EACH STATE
    // EACH CASE HAS A SIMILAR STRUCTURE IN ORDER TO BE FOLLOWED EASILY 
		if (delayCount >= 0) delayCount -- ; // COUNTER IS DECREMENTED
    switch (direction) {
        case CLOCKWISE:  // MOVES THE CURRENT LIT LED FORWARD IN THE ARRAY
						if (delayCount==0){
							ledOnOff(leds[current],LED_OFF);
							current=(current+1)%5; //ASSUMES LED1 IS ON, WHEN THE SYSTEM CHECKS THE DELAY COUNT, AND IT REACHES ZERO, IT TURNS OFF LED1, AND TURNS ON LED2
							ledOnOff(leds[current],LED_ON); // PROCESS REPEATS FROM LED2 TO LED 5 THEN LOOPS BACK TO LED1
							delayCount=delayPeriod;
						}
            else if (pressedB1_ev) {            // B1 HAS BEEN PRESSED
                pressedB1_ev = false ;     
                direction=ANTICLOCKWISE;       // GOES TO NEW STATE
            }
						else if (pressedB5_ev){ /*B5 IS PRESSED THUS CLOCKWISE IS COMMENSED*/
							pressedB5_ev = false;
							invertLEDs(current);
							direction = CLOCKWISE_INVERSE;
            break ;

        case ANTICLOCKWISE: /*MOVES THE CURRENT LIT LED BACKWARD */
						if (delayCount==0){
							ledOnOff(leds[current],LED_OFF);
							current=current-1;
							if(current<0){ //LOGIC IS REVERSED FROM CLOCKWISE, SO LEDS CYCLE BACKWARDS, STARTING AT 5 AND ENDING AT 1
								current=4;
							}
							ledOnOff(leds[current],LED_ON);
							delayCount=delayPeriod;
						}
            else if (pressedB1_ev) {            // B1 HAS BEEN PRESSED
                pressedB1_ev = false ;     // GOES TO NEW STATE
                direction = CLOCKWISE;
            }
						else if (pressedB5_ev){
								pressedB5_ev=false;
								invertLEDs(current);
								direction = ANTICLOCKWISE_INVERSE;
						}
            break ;
				case CLOCKWISE_INVERSE: /*INVERSE OF CLOCKWISE ALL LEDS EXCEPT CURRENT ARE TURNED ON*/
						if (delayCount ==0){
							ledOnOff(leds[current],LED_ON);
							current=(current+1)%5;
							ledOnOff(leds[current],LED_OFF);
							delayCount=delayPeriod;
						}
						else if (pressedB1_ev){
							pressedB1_ev=false;
							direction = ANTICLOCKWISE_INVERSE;
						}
						else if (pressedB5_ev) {            // B5 HAS BEEN PRESSED
                pressedB5_ev = false ;     // ACKNOWLDGES THE EVENT
								invertLEDs(current);
								direction = CLOCKWISE;
		}
						}
				case ANTICLOCKWISE_INVERSE: /*INVERSE OF ANTICLOCKWISE BUT ALL LEDS EXCEPT CURRENT ARE TURNED ON*/
					if (delayCount==0){
							ledOnOff(leds[current],LED_ON);
							current=current-1;
							if(current<0){
								current=4;
							}
							ledOnOff(leds[current],LED_OFF);
							delayCount=delayPeriod;
						}
          else if (pressedB1_ev) {            // B1 HAS BEEN PRESSED
                pressedB1_ev = false ;     
                direction = CLOCKWISE_INVERSE;
            }
					else if (pressedB5_ev){
								pressedB5_ev=false;
								invertLEDs(current);
								direction = ANTICLOCKWISE;
						}
    }
}


/*----------------------------------------------------------------------------
 * Interrupt Handler GPIO E
 *    - Clear the pending request
 *    - Test the bit to see if it generated the interrupt 
  ---------------------------------------------------------------------------- */
void PORTE_IRQHandler(void) {  
    NVIC_ClearPendingIRQ(PORTE_IRQn);
    if ((PORTE->ISFR & B5)) {
        // Add code to respond to interupt here
        pressedB5_ev = true ;
    }
    // Clear status flags 
    PORTE->ISFR = B5 ; 
}



/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
    configureLEDs() ;                 // configure LED1-5 using GPIO
    configureButtons(B1 | B5, true) ; // configures buttons B1 and B5 (with an interrupt)
    initTask1() ;  // initialise task1 state
    initTask2() ;  // initialise task2 state
//    initTask3() ;  // initialise task3 state
    Init_SysTick(1000) ; // initialse SysTick every 1ms
    waitSysTickCounter(10) ;
    while (1) {      // this runs for ever
        task1pollB1() ;
        task2LED() ;
//        task3LED5() ;
        // delay
        waitSysTickCounter(10) ;  // cycle every 10 ms - not explained until week 4
    }
}