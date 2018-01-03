/*
 * UiBase.h
 *
 * Copyright (C) 2015 Martin Lundh
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

#ifndef QUADGS_MODULES_USERINTERFACE_UIBASE_H_
#define QUADGS_MODULES_USERINTERFACE_UIBASE_H_
#include <memory>
#include <vector>
namespace QuadGS {
class QGS_IoInterface;
class QGS_UiCommand;
class QGS_CoreInterface;

/**
 * @class Interface for all Ui implementations.
 */
class QGS_UiInterface
{
public:
	/**
	 * Constructor
	 */
	QGS_UiInterface(){};
    /**
     * Destructor
     */
    virtual ~QGS_UiInterface() {};

    /**
     * Run the UI.
     * @return Return 0 to quit, 1 to continue calling the function.
     */
    virtual bool RunUI() = 0;

    /**
     * Register the commands the implementation provides.
     * @param commands
     */
    virtual void registerCommands(std::vector< std::shared_ptr < QGS_UiCommand >  > commands) = 0;

    /**
     * Bind to a core interface.
     * @param ptr
     */
    virtual void bind(QGS_CoreInterface* ptr) = 0;

    /**
     * Display the text in str to the user.
     * @param str
     */
    virtual void Display(std::string str) = 0;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_USERINTERFACE_UIBASE_H_ */
