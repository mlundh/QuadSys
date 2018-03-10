/*
 * ZedTracker.cpp
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


#include "ZedTracker.h"

#include "QGS_CoreInterface.h"
#include <thread>
#include <functional>
#include <stdio.h>
const int MAX_CHAR_LENG = 128;

namespace QuadGS {

ZedTracker::ZedTracker():mQuit(true)
{
	// Set configuration parameters for the ZED
	InitParameters initParameters;
	initParameters.camera_resolution = RESOLUTION_HD720;
	initParameters.depth_mode = DEPTH_MODE_PERFORMANCE;
	initParameters.coordinate_units = UNIT_METER;
	initParameters.coordinate_system = COORDINATE_SYSTEM_RIGHT_HANDED_Y_UP;

	// Open the camera
	ERROR_CODE err = mZed.open(initParameters);
	if (err != sl::SUCCESS)
	{
		std::cout << sl::errorCode2str(err) << std::endl;
		mZed.close();
	    throw std::runtime_error("Not able to open ZED camera.");
	}

	// Set positional tracking parameters
	mTrackParam.initial_world_transform = sl::Transform::identity();
	mTrackParam.enable_spatial_memory = true;

}

void ZedTracker::RegisterWriteFcn(WriteFcn fcn)
{
	mWriteFcn = fcn;
}

void ZedTracker::msgHandler(std::shared_ptr<QGS_IoHeader> header, std::shared_ptr<QGS_Msg> payload)
{

}

std::vector<std::shared_ptr<QGS_UiCommand>> ZedTracker::getCommands( )
{
	std::vector<std::shared_ptr<QGS_UiCommand>> commands;
	commands.push_back(std::make_shared<QGS_UiCommand> ("trackerStart",
			std::bind(&ZedTracker::startTracking, this, std::placeholders::_1),
			"Start the tracker interface.", QGS_UiCommand::ActOn::NoAction));
	commands.push_back(std::make_shared<QGS_UiCommand> ("trackerStop",
			std::bind(&ZedTracker::stopTracking, this, std::placeholders::_1),
			"Stop the tracker interface.", QGS_UiCommand::ActOn::NoAction));
	return commands;
}

std::string ZedTracker::startTracking(std::string str)
{
	if(mQuit)
	{
		mQuit = false;
		mZed.enableTracking(mTrackParam);
	    mZedThread = std::thread(std::bind(&ZedTracker::run, this));
	}
	return "";
}

std::string ZedTracker::stopTracking(std::string str)
{
	mQuit = true;
	if(mZedThread.joinable())
	{
		mZedThread.join();
	}
	mZed.disableTracking("./AreaFile");
	return "";
}

/**
    This function loops to get image and motion data from the ZED. It is similar to a callback.
    Add your own code here.
 **/
void ZedTracker::run()
{

	// Get the distance between the center of the camera and the left eye
	float translation_left_to_center = mZed.getCameraInformation().calibration_parameters.T.x * 0.5f;

	// Create text for file
	char text_rotation[MAX_CHAR_LENG];
	char text_translation[MAX_CHAR_LENG];
	// Create a CSV file to log motion tracking data
	std::ofstream outputFile;
	std::string csvName = "Motion_data";
	outputFile.open(csvName + ".csv");
	if (!outputFile.is_open())
		std::cout << "WARNING: Can't create CSV file. Run the application with administrator rights." << std::endl;
	else
		outputFile << "Timestamp(ns);Pose Confidence([0,100]);Rotation_X(rad);Rotation_Y(rad);Rotation_Z(rad);Position_X(m);Position_Y(m);Position_Z(m);" << std::endl;


	while ( mZed.getSVOPosition() != (mZed.getSVONumberOfFrames() - 1) && !mQuit)
	{
		if (mZed.grab() == SUCCESS)
		{
			// Get the position of the camera in a fixed reference frame (the World Frame)
			TRACKING_STATE tracking_state = mZed.getPosition(mCameraPose, sl::REFERENCE_FRAME_WORLD);

			if (tracking_state == TRACKING_STATE_OK)
			{
				// getPosition() outputs the position of the Camera Frame, which is located on the left eye of the camera.
				// To get the position of the center of the camera, we transform the pose data into a new frame located at the center of the camera.
				// The generic formula used here is: Pose(new reference frame) = M.inverse() * Pose (camera frame) * M, where M is the transform between two frames.
				transformPose(mCameraPose.pose_data, translation_left_to_center); // Get the pose at the center of the camera (baseline/2 on X axis)


				// Get quaternion, rotation and translation
				//sl::float4 quaternion = mCameraPose.getOrientation();
				sl::float3 rotation = mCameraPose.getEulerAngles(); // Only use Euler angles to display absolute angle values. Use quaternions for transforms.
				sl::float3 translation = mCameraPose.getTranslation();
				// Display translation and rotation (pitch, yaw, roll in OpenGL coordinate system)
				snprintf(text_rotation, MAX_CHAR_LENG, "%3.2f; %3.2f; %3.2f", rotation.x, rotation.y, rotation.z);
				snprintf(text_translation, MAX_CHAR_LENG, "%3.2f; %3.2f; %3.2f", translation.x, translation.y, translation.z);
				// Save the pose data in a csv file
				if (outputFile.is_open())
				{
					outputFile << mZed.getCameraTimestamp() << "; " << mCameraPose.pose_confidence << "; " << text_rotation << "; " << text_translation << ";" << std::endl;
				}
			}

		}
		else sl::sleep_ms(1);
	}
}

/**
 *  Trasnform pose to create a Tracking Frame located in a separate location from the Camera Frame
 **/
void ZedTracker::transformPose(sl::Transform &pose, float tx) {
	sl::Transform transform_;
	transform_.setIdentity();
	// Move the tracking frame by tx along the X axis
	transform_.tx = tx;
	// Apply the transformation
	pose = Transform::inverse(transform_) * pose * transform_;
}

ZedTracker::~ZedTracker()
{
	stopTracking("");
	mZed.close();
}

} /* namespace QuadGS */
