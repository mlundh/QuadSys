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


namespace QuadGS {



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
        last_variable_type  =             7,
    }NodeType_t;

    QuadGSTreeValue(NodeType_t type):
        mNodeType(type),
        mUint8_t(0),
        mUint16_t(0),
        mUint32_t(0),
        mInt8_t(0),
        mInt16_t(0),
        mInt32_t(0)
    {
        if(mNodeType >= last_variable_type)
        {
            throw std::invalid_argument("No such type.");
        }
    }
    virtual ~QuadGSTreeValue()
    {

    }

    std::string SetValue(std::string value)
    {
        try
        {
            int tmp = std::stoi(value);
            switch (mNodeType)
            {
            case NodeType_t::int32_variable_type:
                if(tmp > INT32_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mInt32_t = static_cast<int32_t>(tmp);
                break;
            case NodeType_t::int16_variable_type:
                if(tmp > INT16_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mInt16_t = static_cast<int16_t>(tmp);
                break;
            case NodeType_t::int8_variable_type:
                if(tmp > INT8_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mInt8_t = static_cast<int8_t>(tmp);
                break;
            case NodeType_t::uint32_variable_type:
                if(static_cast<uint32_t>(tmp) > UINT32_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mUint32_t = static_cast<uint32_t>(tmp);
                break;
            case NodeType_t::uint16_variable_type:
                if(tmp > UINT16_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mUint16_t = static_cast<uint16_t>(tmp);
                break;
            case NodeType_t::uint8_variable_type:
                if(tmp > UINT8_MAX)
                {
                    throw std::runtime_error("Set: Overflow. Value not set.");
                }
                mUint8_t = static_cast<uint8_t>(tmp);
                break;
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

    std::string GetValue()
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

    NodeType_t mNodeType;

private:
    uint8_t mUint8_t;
    uint16_t mUint16_t;
    uint32_t mUint32_t;
    int8_t mInt8_t;
    int16_t mInt16_t;
    int32_t mInt32_t;
};







} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREEVALUE_H_ */




