/*
 * Viewer.cpp
 *
 *  Copyright (C) 2018 Martin Lundh
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


#include "Viewer.h"

namespace QuadGS
{

Viewer::Viewer()
{
	// TODO Auto-generated constructor stub

}

Viewer::~Viewer()
{
	// TODO Auto-generated destructor stub
}

std::vector<std::shared_ptr<QGS_UiCommand>> Viewer::getCommands( )
{
	std::vector<std::shared_ptr<QGS_UiCommand>> commands;
	commands.push_back(std::make_shared<QGS_UiCommand> ("viewerStart",
			std::bind(&Viewer::startViewer, this, std::placeholders::_1),
			"Start the viewer.", QGS_UiCommand::ActOn::NoAction));
	commands.push_back(std::make_shared<QGS_UiCommand> ("viewerStop",
			std::bind(&Viewer::stopViewer, this, std::placeholders::_1),
			"Stop the viewer.", QGS_UiCommand::ActOn::NoAction));
	return commands;
}

std::string Viewer::startViewer(std::string str)
{
	mViewerThread = std::thread(std::bind(&ZedTracker::runViewer, this));
	return "";
}

std::string Viewer::stopViewer(std::string str)
{
	closeViewer();
	return "";
}


void Viewer::runViewer()
{
    mViewer.init();
    //glutCloseFunc(std::bind(&ZedTracker::closeViewer, this));
    glutMainLoop();
}


void Viewer::closeViewer()
{
    mViewer.exit();
}

} /* namespace QuadGS */
