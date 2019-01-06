/*
 * Viewer.h
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


#ifndef QUADGS_QGS_VIEWER_VIEWER_VIEWER_H_
#define QUADGS_QGS_VIEWER_VIEWER_VIEWER_H_

namespace QuadGS
{

class Viewer
{
public:
	Viewer();
	virtual ~Viewer();

	std::vector<std::shared_ptr<QGS_UiCommand>> getCommands( );

	std::string startViewer(std::string str);

	std::string stopViewer(std::string str);

	void runViewer();
	void closeViewer();



	TrackingViewer mViewer;
	std::thread mViewerThread;

};

} /* namespace QuadGS */

#endif /* QUADGS_QGS_VIEWER_VIEWER_VIEWER_H_ */
