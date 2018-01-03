/*
 * ZedTracker.h
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

#ifndef QUADGS_QGS_TRACKER_ZED_TRACKER_ZEDTRACKER_H_
#define QUADGS_QGS_TRACKER_ZED_TRACKER_ZEDTRACKER_H_

#include <sl/Camera.hpp>
#include <atomic>
#include "QGS_TrackerInterface.h"

using namespace sl;

namespace QuadGS
{

class ZedTracker : public QGS_TrackerInterface
{
public:
	ZedTracker();
	virtual ~ZedTracker();

	/**
	 * Register the function used to write to the fc.
	 * @param fcn
	 */
	virtual void RegisterWriteFcn(WriteFcn fcn);

	/**
	 * Handle incoming messages.
	 * @param header
	 * @param payload
	 */
	virtual void msgHandler(std::shared_ptr<QGS_MsgHeader> header, std::shared_ptr<QGS_Msg> payload);

	virtual std::vector<std::shared_ptr<QGS_UiCommand>> getCommands( );

	std::string startTracking(std::string str);

	std::string stopTracking(std::string str);
private:

	void run();

	void transformPose(sl::Transform &pose, float tx);




	// ZED objects
	sl::Camera  mZed;
	sl::Pose    mCameraPose;
	std::thread mZedThread;
	TrackingParameters mTrackParam;
	std::atomic<bool>        mQuit;

};

} /* namespace QuadGS */

#endif /* QUADGS_QGS_TRACKER_ZED_TRACKER_ZEDTRACKER_H_ */
