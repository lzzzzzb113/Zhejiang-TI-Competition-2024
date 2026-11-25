#include "ti/devices/msp432e4/driverlib/driverlib.h"
#include <stdint.h>
#include "stdbool.h"

#define BUTTON1_PIN GPIO_PIN_0
#define BUTTON2_PIN GPIO_PIN_1

void buttons_init(void);
uint8_t buttons_read(void);

