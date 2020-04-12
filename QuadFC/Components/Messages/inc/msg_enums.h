/*
 * msg_enums.h
 *
 * Copyright (C) 2019 Martin Lundh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef QUADFC_MESSAGE_ENUMS_H_
#define QUADFC_MESSAGE_ENUMS_H_


typedef enum{
    Control_mode_rate,
    Control_mode_attitude,
    Control_mode_not_available,
}CtrlMode_t;

typedef enum {
    fmode_init,          // FC is initializing.
    fmode_disarmed,      // Disarmed mode, no action except changing to another state.
    fmode_config,        // Configuration. FC can be configured.
    fmode_arming,        // Motors are arming. No action other than change to error, disarmed or armed state.
    fmode_armed,         // Armed state, FC is flight ready.
    fmode_disarming,     // Disarming.
    fmode_fault,         // FC has encountered a serious problem and has halted.
    fmode_not_available, // state information not available.
}FlightMode_t;

typedef enum {
    param_set,
    param_get,
    param_save,
    param_load,
    param_error,
}ParamCtrl;

typedef enum 
{
  QSP_DebugGetRuntimeStats = 0,
  QSP_DebugSetRuntimeStats = 1,
  QSP_DebugGetErrorMessages = 2,
}DbgCtrl_t;

typedef enum 
{
    Name = 0,
    Entry = 1,
    StopAll = 2,
}LogCtrl_t;

typedef enum {
    transmission_OK,
    transmission_NOK,
}TransmissionCtrl;

typedef enum {
    log_name = 0,
    log_entry = 1,
    log_stopAll = 2,
}LogCtrl;

typedef enum {
    getAppLog,
    writeAppLog,
}AppLogCtrl;


#endif /* QUADFC_MESSAGE_ENUMS_H_ */

