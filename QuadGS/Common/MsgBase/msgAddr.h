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
	GS_SerialIO_e,
	FC_Dbg_e,
	FC_SerialIO_e,
	GS_Log_e,
	FC_Log_e,
	FC_Ctrl_e,
	GS_Dbg_e,
	FC_eventSys_e,
	FC_Led_e,
	GS_SetpointGen_e,
	FC_Param_e,
	RC_SetpointGen_e,
	GS_GUI_e,
	GS_Param_e,

	Last_Address
};
typedef enum msgAddr msgAddr_t;

static const std::vector<std::string> msgAddrStr =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"GS_SerialIO",
	"FC_Dbg",
	"FC_SerialIO",
	"GS_Log",
	"FC_Log",
	"FC_Ctrl",
	"GS_Dbg",
	"FC_eventSys",
	"FC_Led",
	"GS_SetpointGen",
	"FC_Param",
	"RC_SetpointGen",
	"GS_GUI",
	"GS_Param",

	"Last_Address",
};
}
#endif /* QUADGS_MESSAGE_ADDRESSES_H_ */
