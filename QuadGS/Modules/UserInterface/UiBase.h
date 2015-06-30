/*
 * UiBase.h
 *
 *  Created on: Apr 22, 2015
 *      Author: martin
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
