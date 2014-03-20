/*
 * debug_functions.c
 *
 * Created: 2013-06-16 18:59:26
 *  Author: Martin Lundh
 */ 
#include "asf.h"

void toggle_pin_internally(int pin)
{
    if (pin == 31)
    {
        gpio_toggle_pin(PIN_31_GPIO);
    }
    else if (pin == 33)
    {
        gpio_toggle_pin(PIN_33_GPIO);
    }
    else if (pin ==35)
    {
        gpio_toggle_pin(PIN_35_GPIO);
    }
}    