/*
 * Group.h
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
#ifndef QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREEVALUE_H_
#define QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREEVALUE_H_

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <cmath>

namespace QuadGS {

#define MAX16f (65536.0)
// y should be MAX16f when using fp16.16
#define INT_TO_FIXED(x,y)         ((x) << (y))
#define DOUBLE_TO_FIXED(x, y)     ((int32_t)(x * y + 0.5))
#define FIXED_TO_INT(x,y)         ((x) >> (y))
#define FIXED_TO_DOUBLE(x, y)     (((double)x) / (y))

class QuadGSTreeValue
{
public:

    typedef enum NodeType
    {
        NoType               =            0,
        uint8_variable_type  =            1,
        uint16_variable_type =            2,
        uint32_variable_type =            3,
        int8_variable_type   =            4,
        int16_variable_type  =            5,
        int32_variable_type  =            6,
        fp_16_16_variable_type =          7,
        last_variable_type  =             8,
    }NodeType_t;

    QuadGSTreeValue(NodeType_t type):
        mNodeType(type),
        mUint8_t(0),
        mUint16_t(0),
        mUint32_t(0),
        mInt8_t(0),
        mInt16_t(0),
        mInt32_t(0),
        mfp_16_16_t(0)
    {
        if(mNodeType >= last_variable_type)
        {
            throw std::invalid_argument("No such type.");
        }
    }
    virtual ~QuadGSTreeValue()
    {

    }

    /**
     * Function to translate a string to a fixed point or integer type.
     * @param value  The string containing the number.
     * @return       the resulting value. Converted to 16.16 if the value contains a decimal point.
     */
    uint32_t StringToIntFixed(std::string value)
    {
        size_t found = value.find('.');
        if(found != std::string::npos)
        {
            std::string decimal_part = value.substr(0,found);
            std::string fractional_part = value.substr(found+1);
            int fractional = std::stoi(fractional_part);
            int decimal    = std::stoi(decimal_part);
            int fractionalLength = fractional_part.length();
            if(fractionalLength < 1)
            {
                throw std::runtime_error("Set: Ill-formated string..");
            }
            if(decimal < 0)
            {
                fractional = -fractional;
            }
            double value_d = ((double)decimal) + ((double)fractional)/(double)(pow(10,fractionalLength));
            int32_t fpVal = DOUBLE_TO_FIXED(value_d, MAX16f);
            if(fpVal > INT32_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            return static_cast<int32_t>(fpVal);
        }
        else
        {
            int tmp = std::stoi(value);
            if(tmp > INT32_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            return static_cast<int32_t>(tmp);
        }
    }
    std::string SetValue(std::string value)
    {
        try
        {
            switch (mNodeType)
            {
            case NodeType_t::fp_16_16_variable_type:
            {
                mfp_16_16_t = StringToIntFixed(value);
                break;
            }
            case NodeType_t::int32_variable_type:
            {
                int tmp = std::stoi(value);
                if(tmp > INT32_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mInt32_t = static_cast<int32_t>(tmp);
                break;
            }
            case NodeType_t::int16_variable_type:
            {
                int tmp = std::stoi(value);
                if(tmp > INT16_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mInt16_t = static_cast<int16_t>(tmp);
                break;
            }
            case NodeType_t::int8_variable_type:
            {
                int tmp = std::stoi(value);
                if(tmp > INT8_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mInt8_t = static_cast<int8_t>(tmp);
                break;
            }
            case NodeType_t::uint32_variable_type:
            {
                int tmp = std::stoi(value);
                if(static_cast<uint32_t>(tmp) > UINT32_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mUint32_t = static_cast<uint32_t>(tmp);
                break;
            }
            case NodeType_t::uint16_variable_type:
            {
                int tmp = std::stoi(value);
                if(tmp > UINT16_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mUint16_t = static_cast<uint16_t>(tmp);
                break;
            }
            case NodeType_t::uint8_variable_type:
            {
                int tmp = std::stoi(value);
                if(tmp > UINT8_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mUint8_t = static_cast<uint8_t>(tmp);
                break;
            }
            case NodeType_t::NoType:
                throw std::runtime_error("Set: Node has no value type.");
            default:
                throw std::runtime_error("Set: Node has no value type.");
                break;
            }
        }
        catch (const std::invalid_argument& e)
        {
            throw e;
        }
        catch (const std::out_of_range& e)
        {
            throw e;
        }
        return "";
    }

    std::string GetValue(bool human_readable = false)
    {
        std::string value;
        try
        {

            switch (mNodeType)
            {
            case NodeType_t::int32_variable_type:
                value = std::to_string(mInt32_t);
                break;
            case NodeType_t::int16_variable_type:
                value = std::to_string(mInt16_t);
                break;
            case NodeType_t::int8_variable_type:
                value = std::to_string(mInt8_t);
                break;
            case NodeType_t::uint32_variable_type:
                value = std::to_string(mUint32_t);
                break;
            case NodeType_t::uint16_variable_type:
                value = std::to_string(mUint16_t);
                break;
            case NodeType_t::uint8_variable_type:
                value = std::to_string(mUint8_t);
                break;
            case NodeType_t::fp_16_16_variable_type:
                if(human_readable)
                {
                    double dVal = FIXED_TO_DOUBLE(mfp_16_16_t, MAX16f);
                    value = std::to_string(dVal);
                }
                else
                {
                    value = std::to_string(mfp_16_16_t);
                }
                break;
            case NodeType_t::NoType:
                throw std::runtime_error("Get: Node has no value type.");
            default:
                throw std::runtime_error("Get: Node has no value.");
                break;
            }
        }
        catch (std::invalid_argument& e)
        {
            throw e;
        }
        catch (std::out_of_range& e)
        {
            throw e;
        }
        return value;
    }

    std::string ModifyValue(std::string value)
    {
        uint32_t incWith = StringToIntFixed(value);
        switch (mNodeType)
        {
        case NodeType_t::fp_16_16_variable_type:
        {
            int tmp = mfp_16_16_t + incWith;
            if(tmp > INT32_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            mfp_16_16_t = static_cast<int32_t>(tmp);
            break;
        }
        case NodeType_t::int32_variable_type:
        {
            int tmp = mInt32_t + incWith;
            if(tmp > INT32_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            mInt32_t = static_cast<int32_t>(tmp);
            break;
        }
        case NodeType_t::int16_variable_type:
        {
            int tmp = mInt16_t + incWith;
            if(tmp > INT16_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            mInt16_t = static_cast<int16_t>(tmp);
            break;
        }
        case NodeType_t::int8_variable_type:
        {
            int tmp = mInt8_t + incWith;
            if(tmp > INT8_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            mInt8_t = static_cast<int8_t>(tmp);
            break;
        }
        case NodeType_t::uint32_variable_type:
        {
            int tmp = mUint32_t + incWith;
            if(static_cast<uint32_t>(tmp) > UINT32_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            mUint32_t = static_cast<uint32_t>(tmp);
            break;
        }
        case NodeType_t::uint16_variable_type:
        {
            int tmp = mUint16_t + incWith;
            if(tmp > UINT16_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            mUint16_t = static_cast<uint16_t>(tmp);
            break;
        }
        case NodeType_t::uint8_variable_type:
        {
            int tmp = mUint8_t + incWith;
            if(tmp > UINT8_MAX)
            {
                throw std::runtime_error("Set: Overflow. Value not set.");
            }
            mUint8_t = static_cast<uint8_t>(tmp);
            break;
        }
        case NodeType_t::NoType:
            throw std::runtime_error("Set: Node has no value type.");
        default:
            throw std::runtime_error("Set: Node has no value type.");
            break;
        }
        return GetValue(true);
    }

    std::string DecValue(std::string value)
    {
        //uint32_t decWith = StringToIntFixed(value);

        return "";
    }

    NodeType_t mNodeType;

private:
    uint8_t mUint8_t;
    uint16_t mUint16_t;
    uint32_t mUint32_t;
    int8_t mInt8_t;
    int16_t mInt16_t;
    int32_t mInt32_t;
    int32_t mfp_16_16_t;
};







} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREEVALUE_H_ */




