/*
 * Group.h
 *
 *  Created on: Apr 26, 2015
 *      Author: martin
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
            int tmp = std::stoi(value);;
            switch (mNodeType)
            {
            case NodeType_t::int32_variable_type:
                mInt32_t = static_cast<int32_t>(tmp);
                break;
            case NodeType_t::int16_variable_type:
                mInt16_t = static_cast<int16_t>(tmp);
                break;
            case NodeType_t::int8_variable_type:
                mInt8_t = static_cast<int8_t>(tmp);
                break;
            case NodeType_t::uint32_variable_type:
                mUint32_t = static_cast<uint32_t>(tmp);
                break;
            case NodeType_t::uint16_variable_type:
                mUint16_t = static_cast<uint16_t>(tmp);
                break;
            case NodeType_t::uint8_variable_type:
                mUint8_t = static_cast<uint8_t>(tmp);
                break;
            case NodeType_t::NoType:
                throw std::runtime_error("Node has no value.");
            default:
                throw std::runtime_error("Node has no value.");
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
                throw std::runtime_error("Node has no value.");
            default:
                throw std::runtime_error("Node has no value.");
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


private:
    bool SerializeUint32(std::vector<uint8_t>& vec)
    {
        if(NodeType_t::int32_variable_type == mNodeType)
        {
            vec.reserve(vec.size() + 4);
            vec.push_back(mUint32_t >> 24);
            vec.push_back(mUint32_t >> 16);
            vec.push_back(mUint32_t >> 8);
            vec.push_back(mUint32_t);
            return true;
        }
        return false;
    }

    bool SerializeUint16(std::vector<uint8_t>& vec)
    {
        if(NodeType_t::uint16_variable_type == mNodeType)
        {
            vec.push_back(mUint16_t >> 8);
            vec.push_back(mUint16_t);
            return true;
        }
        return false;
    }

    bool SerializeUint8(std::vector<uint8_t>& vec)
    {
        if(NodeType_t::uint8_variable_type == mNodeType)
        {
            vec.push_back(mUint8_t);
            return true;
        }
        return false;
    }


    bool DeserializeUint32(uint8_t *buffer)
    {
        if(NodeType_t::uint32_variable_type == mNodeType)
        {
            mUint32_t = 0;

            mUint32_t |= buffer[0] << 24;
            mUint32_t |= buffer[1] << 16;
            mUint32_t |= buffer[2] << 8;
            mUint32_t |= buffer[3] ;
            return true;
        }
        return false;
    }

    bool DeserializeUint16(uint8_t *buffer)
    {
        if(NodeType_t::int32_variable_type == mNodeType)
        {
            mUint16_t = 0;
            mUint16_t |= buffer[0] << 8;
            mUint16_t |= buffer[1] ;
            return true;
        }
        return false;
    }

    bool DeserializeUint8(uint8_t *buffer)
    {
        if(NodeType_t::int32_variable_type == mNodeType)
        {
            mUint8_t = 0;
            mUint8_t = buffer[0];
            return true;
        }
        return false;
    }


public:
    NodeType_t mNodeType;

private:
    uint8_t mUint8_t;
    uint16_t mUint16_t;
    uint32_t mUint32_t;
    uint8_t mInt8_t;
    uint16_t mInt16_t;
    uint32_t mInt32_t;
};







} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREEVALUE_H_ */




