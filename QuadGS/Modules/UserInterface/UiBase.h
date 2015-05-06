/*
 * UiBase.h
 *
 *  Created on: Apr 22, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_USERINTERFACE_UIBASE_H_
#define QUADGS_MODULES_USERINTERFACE_UIBASE_H_
#include <memory>
namespace QuadGS {

class UiBase
{
public:
    typedef std::shared_ptr<UiBase> ptr;
    UiBase();
    virtual ~UiBase();
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_USERINTERFACE_UIBASE_H_ */
