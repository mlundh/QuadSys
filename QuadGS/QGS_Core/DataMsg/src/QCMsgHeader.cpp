/*
 * QCMsgHeader.cpp
 *
 *  Created on: Jan 29, 2017
 *      Author: martin
 */

#include "../../../QGS_Core/DataMsg/QCMsgHeader.h"

#include "../../../QGS_Core/BinaryStream/BinaryStream.h"
namespace QuadGS {


std::vector<std::string> QCMsgHeader::mAddressStrings =
   {
        "Parameters",
        "Log",
        "FunctionCall",
        "Status",
        "Debug"
        "mAddressStrings"
   };

std::vector<std::string> QCMsgHeader::mLogControl =
{
        "Name",
        "Entry",

};

std::vector<std::string> QCMsgHeader::mStatusControl =
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

std::vector<std::string> QCMsgHeader::mTransmissionControl =
{
        "OK",
        "NOK",
};

std::vector<std::string> QCMsgHeader::mDebugControl =
{
        "GetRuntimeStats",
        "SetRuntimeStats",
        "GetErrorMessages",
};

std::vector<std::string> QCMsgHeader::mParamControl =
{
		"SetTree",
		"GetTree",
		"Value",
		"Save",
		"Load"
};

QCMsgHeader::ptr QCMsgHeader::Create(const uint8_t* data, uint16_t length)
{
    ptr p(new QCMsgHeader(data, length) );
    return p;
}
QCMsgHeader::ptr QCMsgHeader::Create(uint8_t Address, uint8_t Control, uint8_t IsResend, uint16_t PayloadSize)
{
    ptr p(new QCMsgHeader(Address, Control, IsResend, PayloadSize) );
    return p;
}

QCMsgHeader::~QCMsgHeader()
{
}

uint8_t QCMsgHeader::GetAddress() const
{
    return mAddress;
}

void QCMsgHeader::SetAddress(uint8_t address)
{
    mAddress = address;
}

uint8_t QCMsgHeader::GetControl() const
{
    return mControl;
}
void QCMsgHeader::SetControl(uint8_t control)
{
    mControl = control;

}

uint8_t QCMsgHeader::GetIsResend() const
{
    return mIsResend;
}


void QCMsgHeader::SetIsResend(uint8_t IsResend)
{
	mIsResend = IsResend;
}

std::string QCMsgHeader::toString() const
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

BinaryOStream& QCMsgHeader::stream(BinaryOStream& os) const
{
	os <<  SetBits(8) << mAddress;
	os <<  SetBits(1) << mIsResend;
	os <<  SetBits(7) << mControl;
	os <<  SetBits(16) << mPayloadSize;
	return os;
}

BinaryIStream& QCMsgHeader::stream(BinaryIStream& is)
{
	is >>  SetBits(8) >> mAddress;
	is >>  SetBits(1) >> mIsResend;
	is >>  SetBits(7) >> mControl;
	is >>  SetBits(16) >> mPayloadSize;
	return is;
}


std::string QCMsgHeader::ControlToString() const
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
    case FunctionCall:{
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

QCMsgHeader::QCMsgHeader(const uint8_t* data, uint16_t data_length)
:QuadGSMsg()
,mAddress(0)
,mControl(0)
,mIsResend(0)
,mPayloadSize(0)
{
	BinaryIStream is(data, data_length);
	is >> *this;
}

QCMsgHeader::QCMsgHeader(uint8_t Address, uint8_t Control, uint8_t IsResend, uint16_t PayloadSize)
:QuadGSMsg()
,mAddress(Address)
,mControl(Control)
,mIsResend(IsResend)
,mPayloadSize(PayloadSize)
{

}

BinaryOStream& operator <<(BinaryOStream& os, const QCMsgHeader& pl)
{
  pl.stream(os);
  return os;
}

BinaryIStream& operator >>(BinaryIStream& is, QCMsgHeader& pl)
{
  pl.stream(is);
  return is;
}


} /* namespace QuadGS */
