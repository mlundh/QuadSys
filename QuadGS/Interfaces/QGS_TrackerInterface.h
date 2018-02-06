/*
 * QGS_Tracker.h
 *
 *  Copyright (C) 2017 Martin Lundh
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

#ifndef QUADGS_QGS_TRACKER_QGS_TRACKERINTERFACE_H_
#define QUADGS_QGS_TRACKER_QGS_TRACKERINTERFACE_H_
#include <memory>
#include <functional>
#include "QGS_Msg.h"
#include "QGS_MsgHeader.h"

namespace QuadGS
{
class QGS_UiCommand;

enum StateNameIf
{
  x_angle_if = 0,
  y_angle_if = 1,
  z_angle_if = 2,
  x_anglVel_if = 3,
  y_anglVel_if = 4,
  z_anglVel_if = 5,
  x_pos_if = 6,
  y_pos_if = 7,
  z_pos_if = 8,
  x_vel_if = 9,
  y_vel_if = 10,
  z_vel_if = 11,
  nr_states_if = 12,
};

enum StateNameBf
{
  pitch_bf = 0,
  roll_bf = 1,
  yaw_bf = 2,
  pitch_rate_bf = 3,
  roll_rate_bf = 4,
  yaw_rate_bf = 5,
  thrust_sp = 6,
  nr_states_bf = 7,

};

struct state_data
{
  double state_bf[nr_states_bf];  //!< body frame state vector.
  uint16_t state_valid_bf;        //!< bits used to determine if the state is valid.
  double state_if[nr_states_if];  //!< Inertial frame state vector.
  uint16_t state_valid_if;        //!< bits used to determine if the state is valid.
};

class QGS_TrackerInterface
{
public:
    typedef std::function<void(std::shared_ptr<QGS_MsgHeader>, std::shared_ptr<QGS_Msg>) > WriteFcn;


	QGS_TrackerInterface(){};
	virtual ~QGS_TrackerInterface() {};

	/**
	 * Write function to send state information to the FC.
	 * @param fcn
	 */
    virtual void RegisterWriteFcn(WriteFcn fcn) = 0;

    /**
     * Handle incoming messages.
     * @param header
     * @param payload
     */
    virtual void msgHandler(std::shared_ptr<QGS_MsgHeader> header, std::shared_ptr<QGS_Msg> payload) = 0;

    /**
     * Get the user commands available for the module.
     * @return  A vector of commands.
     */
    virtual std::vector<std::shared_ptr<QGS_UiCommand>> getCommands( ) = 0;

protected:
    WriteFcn mWriteFcn;

};

} /* namespace QuadGS */

#endif /* QUADGS_QGS_TRACKER_QGS_TRACKERINTERFACE_H_ */
