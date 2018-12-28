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
	FC_Log_e,
	FC_SerialIO_e,
	GUI_e,
	GS_Log_e,
	FC_IO_e,
	FC_Param_e,
	GS_Param_e,

};
typedef enum msgAddr msgAddr_t;

static const std::vector<std::string> msgAddrStr =
{
	"Unassigned",
	"Router",
	"Broadcast",
	"FC_Log",
	"FC_SerialIO",
	"GUI",
	"GS_Log",
	"FC_IO",
	"FC_Param",
	"GS_Param",

};
}
#endif /* QUADGS_MESSAGE_ADDRESSES_H_ */
