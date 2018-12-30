#ifndef QUADGS_MESSAGE_TYPES_H_
#define QUADGS_MESSAGE_TYPES_H_
#include <vector>
#include <string>
namespace QuadGS {

enum messageTypes
{
	Msg_NoType_e,
	Msg_Stop_e,
	Msg_RegisterName_e,
	Msg_Display_e,
	Msg_Test_e,
	Msg_Param_e,
	Msg_Log_e,
	Msg_FindParam_e,
	Msg_GetUiCommands_e,
	Msg_FireUiCommand_e,
	Msg_RegUiCommand_e,
	Msg_UiCommandResult_e,
	Msg_Transmission_e,

	Msg_LastType_e,
};
typedef enum messageTypes messageTypes_t;

static const std::vector<std::string> messageTypesStr =
{
	"Msg_NoType",
	"Msg_Stop",
	"Msg_RegisterName",
	"Msg_Display",
	"Msg_Test",
	"Msg_Param",
	"Msg_Log",
	"Msg_FindParam",
	"Msg_GetUiCommands",
	"Msg_FireUiCommand",
	"Msg_RegUiCommand",
	"Msg_UiCommandResult",
	"Msg_Transmission",

	"Msg_LastType",
};

}
#endif /* QUADGS_MESSAGE_TYPES_H_ */

