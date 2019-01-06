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
	GS_Dbg_e,
	GS_SerialIO_e,
	FC_Log_e,
	FC_Dbg_e,
	GUI_e,
	GS_Log_e,
	FC_Param_e,
	GS_Param_e,
	FC_SerialIO_e,

	Last
};
typedef enum msgAddr msgAddr_t;

static const std::vector<std::string> msgAddrStr =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"GS_Dbg",
	"GS_SerialIO",
	"FC_Log",
	"FC_Dbg",
	"GUI",
	"GS_Log",
	"FC_Param",
	"GS_Param",
	"FC_SerialIO",

	"Last",
};
}
#endif /* QUADGS_MESSAGE_ADDRESSES_H_ */
