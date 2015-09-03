/*
 * IoBase.h
 *
 *  Created on: Apr 22, 2015
 *      Author: martin
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
