/*
 * QSPValueGroup.h
 *
 *  Created on: Apr 18, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_CORE_DATAPACKETS_QSPVALUEGROUP_H_
#define QUADGS_MODULES_CORE_DATAPACKETS_QSPVALUEGROUP_H_

#include <string>
#include <vector>
namespace QuadGS {

class QSPValueGroup
{
public:
    /**
     * @brief Container for name - value pair.
     */
    struct QSPVariable
    {
        QSPVariable(std::string Name, int64_t Value):
            mName(Name),mValue(Value)
        {
        }
        std::string mName;
        int64_t mValue;
    };
    /**
     * @brief enum describing the different variable types
     *             supported by the QSP.
     */
    enum class VariableType
    {
      uint8_variable_type =             1 << 0,
      uint16_variable_type =            1 << 1,
      uint32_variable_type =            1 << 2,
      int8_variable_type =              1 << 3,
      int16_variable_type =             1 << 4,
      int32_variable_type =             1 << 5,
    };

    /**
     * @brief Constructor, initializes the value group with:
     * @param numVariables Number of variables.
     * @param VarType Variable type,
     * @param GroupName Group name, a descriptive string of the variable group.
     */
    QSPValueGroup(int numVariables, VariableType VarType, std::string GroupName);

    /**
     *
     * @param name
     * @param value
     */
    void AddValue(std::string name, int64_t value);

    virtual ~QSPValueGroup();

private:
    int mNumVariables;
    VariableType mVarType;
    std::string mGroupName;
    std::vector< QSPVariable > mVariables;

};


} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_DATAPACKETS_QSPVALUEGROUP_H_ */
