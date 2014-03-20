/*
 * DEBUG_MACROS.h
 *
 * Created: 2013-06-16 18:45:12
 *  Author: Martin Lundh
 */ 


#ifndef DEBUG_MACROS_H_
#define DEBUG_MACROS_H_


#ifdef NDEBUG
    #define toggle_pin(M)
#else
    #define toggle_pin(M) toggle_pin_internally(M)
#endif


void toggle_pin_internally(int pin);

#endif /* DEBUG_MACROS_H_ */