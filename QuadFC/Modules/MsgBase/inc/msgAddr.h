#ifndef QUADFC_MESSAGE_ADDRESSES_H_
#define QUADFC_MESSAGE_ADDRESSES_H_

enum msgAddr
{
	Unassigned,
	Router,
	Broadcast,
	GS_SetpointGen_e,
	GS_Dbg_e,
	GS_SerialIO_e,
	RC_SetpointGen_e,
	GS_GUI_e,
	FC_SerialIO_e,
	FC_Led_e,
	FC_Param_e,
	GS_Log_e,
	FC_Ctrl_e,
	FC_eventSys_e,
	FC_Dbg_e,
	FC_Log_e,
	GS_Param_e,

	Last_Address
};
typedef enum msgAddr msgAddr_t;

static const char* const msgAddrStr[] =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"GS_SetpointGen",
	"GS_Dbg",
	"GS_SerialIO",
	"RC_SetpointGen",
	"GS_GUI",
	"FC_SerialIO",
	"FC_Led",
	"FC_Param",
	"GS_Log",
	"FC_Ctrl",
	"FC_eventSys",
	"FC_Dbg",
	"FC_Log",
	"GS_Param",

	"Last_Address",
};
#endif /* QUADFC_MESSAGE_ADDRESSES_H_ */
