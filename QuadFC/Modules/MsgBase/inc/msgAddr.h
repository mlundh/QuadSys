#ifndef QUADFC_MESSAGE_ADDRESSES_H_
#define QUADFC_MESSAGE_ADDRESSES_H_

enum msgAddr
{
	Unassigned,
	Router,
	Broadcast,
	GS_Dbg_e,
	GS_Log_e,
	GS_GUI_e,
	FC_Dbg_e,
	GS_SerialIO_e,
	RC_SetpointGen_e,
	GS_Param_e,
	FC_Log_e,
	FC_Ctrl_e,
	FC_eventSys_e,
	FC_SerialIO_e,
	GS_SetpointGen_e,
	FC_Led_e,
	FC_Param_e,

	Last_Address
};
typedef enum msgAddr msgAddr_t;

static const char* const msgAddrStr[] =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"GS_Dbg",
	"GS_Log",
	"GS_GUI",
	"FC_Dbg",
	"GS_SerialIO",
	"RC_SetpointGen",
	"GS_Param",
	"FC_Log",
	"FC_Ctrl",
	"FC_eventSys",
	"FC_SerialIO",
	"GS_SetpointGen",
	"FC_Led",
	"FC_Param",

	"Last_Address",
};
#endif /* QUADFC_MESSAGE_ADDRESSES_H_ */
