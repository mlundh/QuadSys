#ifndef QUADFC_MESSAGE_ADDRESSES_H_
#define QUADFC_MESSAGE_ADDRESSES_H_

enum msgAddr
{
	Unassigned,
	Router,
	Broadcast,
	GS_Param_e,
	GS_SerialIO_e,
	GS_GUI_e,
	GS_Log_e,
	FC_SerialIO_e,
	FC_eventSys_e,
	FC_Ctrl_e,
	RC_SetpointGen_e,
	GS_SetpointGen_e,
	FC_Led_e,
	GS_Dbg_e,
	FC_Log_e,
	FC_Param_e,
	FC_Dbg_e,

	Last_Address
};
typedef enum msgAddr msgAddr_t;

static const char* const msgAddrStr[] =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"GS_Param",
	"GS_SerialIO",
	"GS_GUI",
	"GS_Log",
	"FC_SerialIO",
	"FC_eventSys",
	"FC_Ctrl",
	"RC_SetpointGen",
	"GS_SetpointGen",
	"FC_Led",
	"GS_Dbg",
	"FC_Log",
	"FC_Param",
	"FC_Dbg",

	"Last_Address",
};
#endif /* QUADFC_MESSAGE_ADDRESSES_H_ */
