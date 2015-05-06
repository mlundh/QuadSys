/*
 * QSPValueGroup.cpp
 *
 *  Created on: Apr 18, 2015
 *      Author: martin
 */

#include "QSPValueGroup.h"

namespace QuadGS {


QSPValueGroup::QSPValueGroup(int numVariables, VariableType VarType, std::string GroupName):
          mNumVariables(numVariables)
        , mVarType(VarType)
        , mGroupName(GroupName)
        , mVariables()
{

}

QSPValueGroup::~QSPValueGroup()
{

}

void QSPValueGroup::AddValue(std::string name, int64_t value)
{
    QSPVariable tmp(name, value);
    mVariables.push_back(tmp);
}

} /* namespace QuadGS */
