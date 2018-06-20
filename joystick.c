// GPIO joystick demo
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "gpio_v2.h"
#include "hw_types.h"      // For HWREG(...) macro
#include "hw_cm_per.h"
#include "watchdog.h"
#include "leds.h"
#include "joystick.h"
#include <stdint.h>

#define BUTTON_GPIO_BASE     (SOC_GPIO_2_REGS)
#define BUTTON_PIN           (1)

#define DELAY_TIME 0x4000000

#define BAUD_RATE_115200          (115200)
#define UART_MODULE_INPUT_CLK     (48000000)

static void initializeButtonPin(void);

void Joystick_init(){
	initializeButtonPin();
}

_Bool Joystick_readLeftWithStarteWare(void){
	return GPIOPinRead(BUTTON_GPIO_BASE, BUTTON_PIN) == 0;
}

static void GPIO2ModuleClkConfig(void)
{

    /* Writing to MODULEMODE field of CM_PER_GPIO1_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_MODULEMODE));
    /*
    ** Writing to OPTFCLKEN_GPIO_2_GDBCLK bit in CM_PER_GPIO2_CLKCTRL
    ** register.
    */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK;

    /*
    ** Waiting for OPTFCLKEN_GPIO_1_GDBCLK bit to reflect the desired
    ** value.
    */
    while(CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK));

    /*
    ** Waiting for IDLEST field in CM_PER_GPIO2_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_GPIO2_CLKCTRL_IDLEST_FUNC <<
           CM_PER_GPIO2_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
            CM_PER_GPIO2_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_GPIO_2_GDBCLK bit in CM_PER_L4LS_CLKSTCTRL
    ** register to attain desired value.
    */
    while(CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK));
}

static void initializeButtonPin(void){
    /* Enabling functional clocks for GPIO2 instance. */
	GPIO2ModuleClkConfig();

    /* Selecting GPIO1[23] pin for use. */
    //GPIO1Pin23PinMuxSetup();

    /* Enabling the GPIO module. */
    GPIOModuleEnable(BUTTON_GPIO_BASE);

    /* Resetting the GPIO module. */
    GPIOModuleReset(BUTTON_GPIO_BASE);

    /* Setting the GPIO pin as an input pin. */
    GPIODirModeSet(BUTTON_GPIO_BASE,
                   BUTTON_PIN,
                   GPIO_DIR_INPUT);
}

