#ifndef QUADFC_MESSAGE_ADDRESSES_H_
#define QUADFC_MESSAGE_ADDRESSES_H_

enum msgAddr
{
	Unassigned,
	Router,
	Broadcast,
	FC_SerialIO_e,
	FC_Log_e,
	FC_Led_e,
	GS_Dbg_e,
	GS_Param_e,
	GS_SetpointGen_e,
	RC_SetpointGen_e,
	GS_GUI_e,
	FC_Dbg_e,
	FC_Param_e,
	GS_SerialIO_e,
	FC_Ctrl_e,
	GS_Log_e,
	FC_eventSys_e,

	Last_Address
};
typedef enum msgAddr msgAddr_t;

static const char* const msgAddrStr[] =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"FC_SerialIO",
	"FC_Log",
	"FC_Led",
	"GS_Dbg",
	"GS_Param",
	"GS_SetpointGen",
	"RC_SetpointGen",
	"GS_GUI",
	"FC_Dbg",
	"FC_Param",
	"GS_SerialIO",
	"FC_Ctrl",
	"GS_Log",
	"FC_eventSys",

	"Last_Address",
};
#endif /* QUADFC_MESSAGE_ADDRESSES_H_ */
