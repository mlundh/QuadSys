#ifndef QUADGS_MESSAGE_ADDRESSES_H_
#define QUADGS_MESSAGE_ADDRESSES_H_
#include <vector>
#include <string>
namespace QuadGS {

enum msgAddr
{
	Unassigned,
	Router,
	Broadcast,
	FC_Param_e,
	GS_SerialIO_e,
	FC_SerialIO_e,
	FC_Dbg_e,
	GS_Param_e,
	FC_Log_e,
	FC_Led_e,
	GS_SetpointGen_e,
	RC_SetpointGen_e,
	FC_Ctrl_e,
	GS_Log_e,
	GS_GUI_e,
	GS_Dbg_e,
	FC_eventSys_e,

	Last_Address
};
typedef enum msgAddr msgAddr_t;

static const std::vector<std::string> msgAddrStr =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"FC_Param",
	"GS_SerialIO",
	"FC_SerialIO",
	"FC_Dbg",
	"GS_Param",
	"FC_Log",
	"FC_Led",
	"GS_SetpointGen",
	"RC_SetpointGen",
	"FC_Ctrl",
	"GS_Log",
	"GS_GUI",
	"GS_Dbg",
	"FC_eventSys",

	"Last_Address",
};
}
#endif /* QUADGS_MESSAGE_ADDRESSES_H_ */
