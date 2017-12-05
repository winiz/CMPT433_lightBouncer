// Fake Typing bare metal sample application
// On the serial port, fakes

#include "consoleUtils.h"
#include <stdint.h>

// My hardware abstraction modules
#include "serial.h"
#include "timer.h"
#include "wdtimer.h"
#include "leds.h"

// My application's modules
#include "lightBouncer.h"


/******************************************************************************
 **              JOKE DATA
 ******************************************************************************/

#define NUM_JOKES 3
static int s_curJoke = 0;
static char* jokes[] = {
		(char[]){0x53, 0x69, 0x67, 0x6E, 0x20, 0x6F, 0x6E, 0x20, 0x64, 0x6F, 0x6F,
		         0x72, 0x20, 0x6F, 0x66, 0x20, 0x68, 0x61, 0x63, 0x6B, 0x65, 0x72,
		         0x2E, 0x2E, 0x2E, 0x20, 0x20, 0x20, 0x27, 0x47, 0x6F, 0x6E, 0x65,
		         0x20, 0x50, 0x68, 0x69, 0x73, 0x68, 0x69, 0x6E, 0x67, 0x27, 0x0A,
		         0x0D, 0x00},
		(char[]){0x49, 0x20, 0x73, 0x74, 0x61, 0x72, 0x74, 0x65, 0x64, 0x20, 0x61,
		         0x20, 0x62, 0x61, 0x6E, 0x64, 0x20, 0x6E, 0x61, 0x6D, 0x65, 0x64,
		         0x20, 0x27, 0x31, 0x30, 0x32, 0x33, 0x4D, 0x42, 0x27, 0x2C, 0x20,
		         0x62, 0x75, 0x74, 0x20, 0x63, 0x6F, 0x75, 0x6C, 0x64, 0x20, 0x6E,
		         0x65, 0x76, 0x65, 0x72, 0x20, 0x67, 0x65, 0x74, 0x20, 0x61, 0x20,
		         0x67, 0x69, 0x67, 0x2E, 0x0A, 0x0D, 0x00},
		(char[]){0x4D, 0x79, 0x20, 0x63, 0x6F, 0x6D, 0x70, 0x75, 0x74, 0x65, 0x72,
		         0x20, 0x69, 0x73, 0x20, 0x73, 0x6F, 0x20, 0x73, 0x6C, 0x6F, 0x77,
		         0x20, 0x69, 0x74, 0x20, 0x68, 0x65, 0x72, 0x74, 0x7A, 0x2E, 0x0A,
		         0x0D, 0x00}
};

static void serialRxIsrCallback(uint8_t rxByte);
static void doBackgroundSerialWork(void);
static void listCommands(void);

/******************************************************************************
 **              SERIAL PORT HANDLING
 ******************************************************************************/
static volatile uint8_t s_rxByte = 0;
static void serialRxIsrCallback(uint8_t rxByte) {
	s_rxByte = rxByte;
}

static void doBackgroundSerialWork(void)
{
	if (s_rxByte != 0) {
		// Tell a joke
		if (s_rxByte == 'j') {
			ConsoleUtilsPrintf("\nNow queuing a joke...\n");
			LightBouncer_setMessage(jokes[s_curJoke]);
			s_curJoke = (s_curJoke + 1) % NUM_JOKES;
		}

		else if (s_rxByte == '?') {
			listCommands();
		}

		else if(s_rxByte == 'a' || s_rxByte == 'A') {
			ConsoleUtilsPrintf("bounce mode enabled\n");
			//Leds_setMode(MODE_A);
		}
		else if(s_rxByte == 'x' || s_rxByte == 'X') {
			ConsoleUtilsPrintf("No longer hitting the watchdog.\n");
			Timer_stopHitingWD();
		}

		s_rxByte = 0;
	}
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

static void printWelcomeMessage(void){
	ConsoleUtilsPrintf("\nWelcome to Light Bouncer, Hope you enjoy the party! \n");
	ConsoleUtilsPrintf("    Written by William Xinran Zhang.\n");
	ConsoleUtilsPrintf("------------------------------------------------\n");
}

/******************************************************************************
 **              MAIN
 ******************************************************************************/
int main(void)
{
	// Initialization
	Serial_init(serialRxIsrCallback);
	Timer_init();
	Watchdog_init();
	LightBouncer_init();

	// Setup callbacks from hardware abstraction modules to application:
	Serial_setRxIsrCallback(serialRxIsrCallback);
	Timer_setTimerIsrCallback(LightBouncer_notifyOnTimeIsr);

	// Display startup messages to console:
	printWelcomeMessage();
	listCommands();

	// Main loop:
	while(1) {
		// Handle background processing
		doBackgroundSerialWork();
		LightBouncer_doBackgroundWork();
		//driveLedsWithSetAndClear();

		// Timer ISR signals intermittent background activity.
		if(Timer_isIsrFlagSet()) {
			Timer_clearIsrFlag();
			if (Timers_getWDhittingState()){
				Watchdog_hit();
			}
		}
	}
}
