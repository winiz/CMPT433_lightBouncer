// lightBouncer application
// On the serial port, Bounce or Bar light

#include "consoleUtils.h"
#include "hw_types.h" 
#include <stdint.h>

// My hardware abstraction modules
#include "serial.h"
#include "timer.h"
#include "wdtimer.h"
#include "leds.h"
#include "joystick.h"

#define EXTERNAL_RST 5
#define WDT_RST      4
#define COLD_RST   	 0

#define RESET_BASE 0x44E00F00
#define RESET_OFFSET 0x8

static void serialRxIsrCallback(uint8_t rxByte);
static void doBackgroundSerialWork(void);
static void listCommands(void);
static void printResetScources(void);
static void printWelcomeMessage(void);

static volatile uint8_t s_rxByte = 0;

/******************************************************************************
 **              MAIN
 ******************************************************************************/
int main(void)
{

	// Initialization
	Serial_init(serialRxIsrCallback);
	Timer_init();
	Watchdog_init();
	Leds_init();
	Joystick_init();

	// Setup callbacks from hardware abstraction modules to application:
	Serial_setRxIsrCallback(serialRxIsrCallback);

	// Display startup messages to console:
	printWelcomeMessage();
	printResetScources();
	listCommands();

	_Bool lastButtonState = false;
	// Main loop:
	while(1) {
		_Bool isButtonPressed = Joystick_readLeftWithStarteWare();

		// Handle background processing
		doBackgroundSerialWork();
		

		if (isButtonPressed){
			Leds_toggleMode();
		}

		if (lastButtonState != isButtonPressed) {
			ConsoleUtilsPrintf("Hint: Toggle only works if you press left 2s then release. \n");
			lastButtonState = isButtonPressed;
		}

		// Timer ISR signals intermittent background activity.
		if(Timer_isIsrFlagSet()) {
			Timer_clearIsrFlag();
			if (Timers_getWDhittingState()){
				Watchdog_hit();
			}
		}
	}
}




/******************************************************************************
 **              SERIAL PORT HANDLING
 ******************************************************************************/
static void serialRxIsrCallback(uint8_t rxByte) {
	s_rxByte = rxByte;
}

static void doBackgroundSerialWork(void)
{	
	Leds_flashing();
	if (s_rxByte != 0) {
		if (s_rxByte == '?') {
			listCommands();
		}
		else if(s_rxByte >= 48 && s_rxByte <= 57) {
			ConsoleUtilsPrintf("\nSetting LED speed to %c\n", s_rxByte);
			Leds_changeSpeed(s_rxByte - 48);
		}
		else if(s_rxByte == 'a' || s_rxByte == 'A') {
			ConsoleUtilsPrintf("\nChanging to bounce mode\n");
			Leds_setMode(BOUNCE);
		}
		else if(s_rxByte == 'b' || s_rxByte == 'B') {
			ConsoleUtilsPrintf("\nChanging to bar mode\n");
			Leds_setMode(BAR);
		}
		else if(s_rxByte == 'x' || s_rxByte == 'X') {
			ConsoleUtilsPrintf("\nNo longer hitting the watchdog.\n");
			Timer_stopHitingWD();
		}
		else {
			ConsoleUtilsPrintf("\nInvalid command\n");
		}

		s_rxByte = 0;
	}
}

static void printWelcomeMessage(void){
	ConsoleUtilsPrintf("\nWelcome to Light Bouncer, Hope you enjoy the party! \n");
	ConsoleUtilsPrintf("    Written by William Xinran Zhang.\n");
	ConsoleUtilsPrintf("------------------------------------------------\n");
}

static void printResetScources(void){

	uint32_t resetSourceRegister = HWREG(RESET_BASE + RESET_OFFSET);

	ConsoleUtilsPrintf("Reset source (0x%x) = ", resetSourceRegister);

	if((resetSourceRegister & (1 << EXTERNAL_RST)) != 0) {
		ConsoleUtilsPrintf("External reset, \n");
	}
	else if((resetSourceRegister & (1 << WDT_RST)) != 0) {
		ConsoleUtilsPrintf("Watchdog reset, \n");
	}
	else if((resetSourceRegister & (1 << COLD_RST)) != 0) {
		ConsoleUtilsPrintf("Cold reset, \n");
	}

	// clean up 
	HWREG(RESET_BASE + RESET_OFFSET) |= resetSourceRegister;
}

static void listCommands(void){
	ConsoleUtilsPrintf(" \n");
	ConsoleUtilsPrintf("Commands:\n");
	ConsoleUtilsPrintf(" '?' : Display help message.\n");
	ConsoleUtilsPrintf(" '0-9' : Set speed 0 (slow) to 9 (fast).\n");
	ConsoleUtilsPrintf(" 'a':  Select pattern A (bounce).\n");
	ConsoleUtilsPrintf(" 'b':  Select pattern B (bar).\n");
	ConsoleUtilsPrintf(" 'x':  Stop hitting the watchdog.\n");
	ConsoleUtilsPrintf(" 'BTN':  Push-button to toggle mode.\n");
}





