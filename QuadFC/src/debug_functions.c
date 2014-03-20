/*
 * debug_functions.c
 *
 * Copyright (C) 2014  Martin Lundh
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
