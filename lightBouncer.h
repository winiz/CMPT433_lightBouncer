// LightBouncer.h:
// Application module to simulate typing on the screen.

#ifndef _LIGHTBOUNCER_H_
#define _LIGHTBOUNCER_H_

void LightBouncer_init(void);
void LightBouncer_setMessage(const char *newMessage);
void LightBouncer_notifyOnTimeIsr(void);
void LightBouncer_doBackgroundWork(void);


#endif
