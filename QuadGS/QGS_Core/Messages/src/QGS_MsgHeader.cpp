/*
 * QCMsgHeader.cpp
 *
 *  Created on: Jan 29, 2017
 *      Author: martin
 */

#include "BinaryStream.h"
#include "QGS_MsgHeader.h"
namespace QuadGS {


std::vector<std::string> QGS_MsgHeader::mAddressStrings =
   {
        "Parameters",
        "Log",
        "FunctionCall",
        "Status",
        "Debug"
        "mAddressStrings"
   };

std::vector<std::string> QGS_MsgHeader::mLogControl =
{
        "Name",
        "Entry",

};

std::vector<std::string> QGS_MsgHeader::mStatusControl =
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

std::vector<std::string> QGS_MsgHeader::mTransmissionControl =
{
        "OK",
        "NOK",
};

std::vector<std::string> QGS_MsgHeader::mDebugControl =
{
        "GetRuntimeStats",
        "SetRuntimeStats",
        "GetErrorMessages",
};

std::vector<std::string> QGS_MsgHeader::mParamControl =
{
		"SetTree",
		"GetTree",
		"Value",
		"Save",
		"Load"
};

QGS_MsgHeader::ptr QGS_MsgHeader::Create(const uint8_t* data, uint16_t length)
{
    ptr p(new QGS_MsgHeader(data, length) );
    return p;
}
QGS_MsgHeader::ptr QGS_MsgHeader::Create(uint8_t Address, uint8_t Control, uint8_t IsResend, uint16_t PayloadSize)
{
    ptr p(new QGS_MsgHeader(Address, Control, IsResend, PayloadSize) );
    return p;
}

QGS_MsgHeader::~QGS_MsgHeader()
{
}

uint8_t QGS_MsgHeader::GetAddress() const
{
    return mAddress;
}

void QGS_MsgHeader::SetAddress(uint8_t address)
{
    mAddress = address;
}

uint8_t QGS_MsgHeader::GetControl() const
{
    return mControl;
}
void QGS_MsgHeader::SetControl(uint8_t control)
{
    mControl = control;

}

uint8_t QGS_MsgHeader::GetIsResend() const
{
    return mIsResend;
}


void QGS_MsgHeader::SetIsResend(uint8_t IsResend)
{
	mIsResend = IsResend;
}

std::string QGS_MsgHeader::toString() const
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

BinaryOStream& QGS_MsgHeader::stream(BinaryOStream& os) const
{
	os <<  SetBits(8) << mAddress;
	os <<  SetBits(1) << mIsResend;
	os <<  SetBits(7) << mControl;
	os <<  SetBits(16) << mPayloadSize;
	return os;
}

BinaryIStream& QGS_MsgHeader::stream(BinaryIStream& is)
{
	is >>  SetBits(8) >> mAddress;
	is >>  SetBits(1) >> mIsResend;
	is >>  SetBits(7) >> mControl;
	is >>  SetBits(16) >> mPayloadSize;
	return is;
}


std::string QGS_MsgHeader::ControlToString() const
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

QGS_MsgHeader::QGS_MsgHeader(const uint8_t* data, uint16_t data_length)
:QGS_Msg()
,mAddress(0)
,mControl(0)
,mIsResend(0)
,mPayloadSize(0)
{
	BinaryIStream is(data, data_length);
	is >> *this;
}

QGS_MsgHeader::QGS_MsgHeader(uint8_t Address, uint8_t Control, uint8_t IsResend, uint16_t PayloadSize)
:QGS_Msg()
,mAddress(Address)
,mControl(Control)
,mIsResend(IsResend)
,mPayloadSize(PayloadSize)
{

}

BinaryOStream& operator <<(BinaryOStream& os, const QGS_MsgHeader& pl)
{
  pl.stream(os);
  return os;
}

BinaryIStream& operator >>(BinaryIStream& is, QGS_MsgHeader& pl)
{
  pl.stream(is);
  return is;
}


} /* namespace QuadGS */
