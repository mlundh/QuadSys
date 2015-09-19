/*
 * IoBase.h
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

#ifndef QUADGS_MODULES_USERINTERFACE_IOBASE_H_
#define QUADGS_MODULES_USERINTERFACE_IOBASE_H_
#include <memory>
namespace QuadGS {

class Core;
class Command;
class QuadSerialPacket;
class QspPayloadRaw;

class IoBase
{
public:
  /**
  * @brief Callback typedefs.
  */
  typedef std::function<void( std::shared_ptr<QuadSerialPacket> )> MessageHandlerFcn;
  typedef std::function<void( std::shared_ptr<QspPayloadRaw> )> MessageHandlerRawFcn;
  typedef std::function<void( void )> TimeoutHandlerFcn;

  typedef std::shared_ptr<IoBase> ptr;
  IoBase(){}
  virtual ~IoBase(){}
  virtual void write( std::shared_ptr<QspPayloadRaw> ptr) = 0;
  virtual void startRead( void ) = 0;
  virtual void setReadCallback( MessageHandlerFcn ) = 0;
  virtual std::vector< std::shared_ptr<Command> > getCommands( ) = 0;
  virtual std::string getStatus( ) = 0;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_USERINTERFACE_IOBASE_H_ */
