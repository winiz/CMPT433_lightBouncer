// leds.h:
// Hardware  module: led controller

#ifndef LEDS_H_
#define LEDS_H_

#include <stdint.h>

#define BOUNCE 1
#define BAR 2

void Leds_init(void);
void Leds_flashing(void);
void Leds_setMode(uint8_t requestedMode);
void Leds_changeSpeed(uint8_t requestedSpeed);
uint8_t Leds_getSpeed(void);
void Leds_toggleMode(void);

#endif /* LEDS_H_ */
