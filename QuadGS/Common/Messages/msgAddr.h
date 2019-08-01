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
	GS_SetpointGen_e,
	FC_eventSys_e,
	FC_Param_e,
	GS_Log_e,
	GS_Param_e,
	FC_Led_e,
	FC_Dbg_e,
	RC_SetpointGen_e,
	FC_Log_e,
	GS_Dbg_e,
	GS_SerialIO_e,
	FC_Ctrl_e,
	FC_SerialIO_e,
	GS_GUI_e,

	Last_Address
};
typedef enum msgAddr msgAddr_t;

static const std::vector<std::string> msgAddrStr =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"GS_SetpointGen",
	"FC_eventSys",
	"FC_Param",
	"GS_Log",
	"GS_Param",
	"FC_Led",
	"FC_Dbg",
	"RC_SetpointGen",
	"FC_Log",
	"GS_Dbg",
	"GS_SerialIO",
	"FC_Ctrl",
	"FC_SerialIO",
	"GS_GUI",

	"Last_Address",
};
}
#endif /* QUADGS_MESSAGE_ADDRESSES_H_ */
