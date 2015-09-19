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

class UiBase
{
public:
    typedef std::shared_ptr<UiBase> ptr;
    UiBase(){};
    virtual ~UiBase(){};
    virtual void bind(std::shared_ptr<IoBase> IoPtr) = 0;
    virtual bool RunUI() = 0;
    virtual void registerCommands(std::vector< Command::ptr > commands) = 0;
    virtual void SetCore(Core::ptr ptr) = 0;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_USERINTERFACE_UIBASE_H_ */
