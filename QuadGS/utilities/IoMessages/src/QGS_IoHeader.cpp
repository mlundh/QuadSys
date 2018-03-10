/*
 * QGS_IoHeader.cpp
 *
 *  Created on: Jan 29, 2017
 *      Author: martin
 */

#include "QGS_IoHeader.h"

#include "BinaryStream.h"
namespace QuadGS {


std::vector<std::string> QGS_IoHeader::mAddressStrings =
   {
        "Parameters",
        "Log",
        "FunctionCall",
        "Status",
        "Debug"
        "mAddressStrings"
   };

std::vector<std::string> QGS_IoHeader::mLogControl =
{
        "Name",
        "Entry",

};

std::vector<std::string> QGS_IoHeader::mStatusControl =
{
        "Cont",
        "Ack",
        "Nack",
        "Error",
        "NotAllowed",
        "UnexpectedSequence",
        "NotValidSlipPacket",
        "BufferOverrun"
};

std::vector<std::string> QGS_IoHeader::mTransmissionControl =
{
        "OK",
        "NOK",
};

std::vector<std::string> QGS_IoHeader::mDebugControl =
{
        "GetRuntimeStats",
        "SetRuntimeStats",
        "GetErrorMessages",
};

std::vector<std::string> QGS_IoHeader::mParamControl =
{
		"SetTree",
		"GetTree",
		"Value",
		"Save",
		"Load"
};


QGS_IoHeader::QGS_IoHeader(const QGS_IoHeader& header)
:QGS_Msg()
,mAddress(header.GetAddress())
,mControl(header.GetControl())
,mIsResend(header.GetIsResend())
,mPayloadSize(header.GetPayloadSize())
{

}

QGS_IoHeader::QGS_IoHeader(const uint8_t* data, uint16_t data_length)
:QGS_Msg()
,mAddress(0)
,mControl(0)
,mIsResend(0)
,mPayloadSize(0)
{
	BinaryIStream is(data, data_length);
	is >> *this;
}

QGS_IoHeader::QGS_IoHeader(uint8_t Address, uint8_t Control, uint8_t IsResend, uint16_t PayloadSize)
:QGS_Msg()
,mAddress(Address)
,mControl(Control)
,mIsResend(IsResend)
,mPayloadSize(PayloadSize)
{

}

QGS_IoHeader::~QGS_IoHeader()
{
}

uint8_t QGS_IoHeader::GetAddress() const
{
    return mAddress;
}

void QGS_IoHeader::SetAddress(uint8_t address)
{
    mAddress = address;
}

uint8_t QGS_IoHeader::GetControl() const
{
    return mControl;
}
void QGS_IoHeader::SetControl(uint8_t control)
{
    mControl = control;

}

uint8_t QGS_IoHeader::GetIsResend() const
{
    return mIsResend;
}



void QGS_IoHeader::SetIsResend(uint8_t IsResend)
{
	mIsResend = IsResend;
}


uint8_t QGS_IoHeader::GetPayloadSize() const
{
    return mPayloadSize;
}

std::string QGS_IoHeader::toString() const
{
    std::string result;
    uint8_t address = GetAddress();
    if (static_cast< std::size_t >(address) < mAddressStrings.size())
    {
        result += "[" + mAddressStrings[address] + "]";
    }
    else
    {
        result += "[Unknown address]: " + std::to_string(address) + " With control: " + std::to_string(GetControl());
        return result;
    }

    std::string is_resend = GetIsResend() ? "IsResend" : "NotResend";
    result += "[" + is_resend + "]";

    result += ControlToString();
    result += "[Payload Size:" + std::to_string(mPayloadSize) +"]";
    return result;
}

BinaryOStream& QGS_IoHeader::stream(BinaryOStream& os) const
{
	os <<  SetBits(8) << mAddress;
	os <<  SetBits(1) << mIsResend;
	os <<  SetBits(7) << mControl;
	os <<  SetBits(16) << mPayloadSize;
	return os;
}

BinaryIStream& QGS_IoHeader::stream(BinaryIStream& is)
{
	is >>  SetBits(8) >> mAddress;
	is >>  SetBits(1) >> mIsResend;
	is >>  SetBits(7) >> mControl;
	is >>  SetBits(16) >> mPayloadSize;
	return is;
}


std::string QGS_IoHeader::ControlToString() const
{


    std::vector<std::string>* tmp;

    switch(mAddress)
    {
    case Parameters:{
    	tmp = &mParamControl;
    	break;
    }
    case Log:{
    	tmp = &mLogControl;
    	break;
    }
    case State:{
    	tmp = NULL;
    	break;
    }
    case Status:{
    	tmp = &mStatusControl;
    	break;
    }
    case Debug:{
    	tmp = &mDebugControl;
    	break;
    }
    case Transmission:{
        tmp = &mTransmissionControl;
        break;
    }
    default:
    	tmp = NULL;

    }
    int8_t status = GetControl();
    std::string result;

    if ((tmp!=NULL) && static_cast< std::size_t >(status) < tmp->size())
    {
        result += "[" + (*tmp)[status] + "]";
    }
    else
    {
        result += "[Unknown control]: " + std::to_string(status);
    }
    return result;
}


} /* namespace QuadGS */
