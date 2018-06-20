// GPIO LED demo
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "gpio_v2.h"
#include "hw_types.h"      // For HWREG(...) macro
#include "watchdog.h"
#include "leds.h"
#include "timer.h"
#include <stdint.h>


/*****************************************************************************
 **                INTERNAL MACRO DEFINITIONS
 *****************************************************************************/
#define LED_GPIO_BASE           (SOC_GPIO_1_REGS)
#define LED0_PIN (21)
#define LED1_PIN (22)
#define LED2_PIN (23)
#define LED3_PIN (24)

#define LED_MASK ((1<<LED0_PIN) | (1<<LED1_PIN) | (1<<LED2_PIN) | (1<<LED3_PIN))

#define MAX_SPEED 9
#define DEFAULT_SPEED 8
#define MIN_SPEED 0

//#define DELAY_TIME 0x4000000		// Delay with MMU enabled
#define DELAY_TIME 0x40000		// Delay witouth MMU and cache
static uint8_t speed = DEFAULT_SPEED;
static uint8_t mode = BOUNCE;
static uint32_t flashingTime;
/*****************************************************************************
 **                INTERNAL FUNCTION PROTOTYPES
 *****************************************************************************/
static void flashTimePeriod(void);
static void bounce(void);
static void bar(void);
static void busyWait(volatile unsigned int count);


void Leds_init(void)
{
	/* Enabling functional clocks for GPIO1 instance. */
	GPIO1ModuleClkConfig();

	/* Selecting GPIO1[23] pin for use. */
	//GPIO1Pin23PinMuxSetup();

	/* Enabling the GPIO module. */
	GPIOModuleEnable(LED_GPIO_BASE);

	/* Resetting the GPIO module. */
	GPIOModuleReset(LED_GPIO_BASE);

	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_GPIO_BASE,
			LED0_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED1_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED2_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED3_PIN,
			GPIO_DIR_OUTPUT);
	flashTimePeriod();
}

void Leds_flashing(void){
	if(mode == BOUNCE) {
		bounce();
	}
	else if(mode == BAR){
		bar();
	}
}

void Leds_setMode(uint8_t requestedMode){
	mode = requestedMode;
}

void Leds_changeSpeed(uint8_t requestedSpeed){
	speed = requestedSpeed;
	flashTimePeriod();
}

uint8_t Leds_getSpeed(void){
	return speed;
}

void Leds_toggleMode(void){
	if(mode == BOUNCE){
		Leds_setMode(BAR);
	} else if(mode == BAR){
		Leds_setMode(BOUNCE);
	}
}

/*****************************************************************************
 **                INTERNAL FUNCTION DEFINITION
 *****************************************************************************/

static void bounce(void)
{
		// Flash each LED individually
		for (int pin = LED0_PIN; pin <= LED2_PIN; pin++) {
			/* Driving a logic HIGH on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,pin,GPIO_PIN_HIGH);

			busyWait(DELAY_TIME);

			/* Driving a logic LOW on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,pin,GPIO_PIN_LOW);

			busyWait(DELAY_TIME);
		}

		for (int pin = LED3_PIN; pin >= LED0_PIN; pin--) {
			/* Driving a logic HIGH on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,pin,GPIO_PIN_HIGH);

			busyWait(DELAY_TIME);

			/* Driving a logic LOW on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,pin,GPIO_PIN_LOW);

			busyWait(DELAY_TIME);
		}

}


static void bar(void){

		for (int pin = LED0_PIN; pin <= LED3_PIN; pin++) {
			HWREG(LED_GPIO_BASE + GPIO_SETDATAOUT) = 1<<pin;
			busyWait(DELAY_TIME);
		}
		for (int pin = LED3_PIN; pin >= LED0_PIN; pin--) {
			HWREG(LED_GPIO_BASE + GPIO_CLEARDATAOUT) = 1<<pin;
			busyWait(DELAY_TIME);
		}
}

static void busyWait(volatile unsigned int count)
{	
	uint32_t newCount = count/flashingTime;
	while((newCount--))
		;
}

static void flashTimePeriod(void){
	uint8_t i;
	uint32_t speedDividerFactor = 1;
	for(i = 0; i < MAX_SPEED - speed; i++) {
		speedDividerFactor = speedDividerFactor * 2;
	}
	flashingTime = speedDividerFactor;
}