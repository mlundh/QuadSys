#ifndef QUADGS_MESSAGE_TYPES_H_
#define QUADGS_MESSAGE_TYPES_H_
namespace QuadGS {

enum messageTypes
{
	Msg_Stop_e,
	Msg_DebugIo_e,
	Msg_ParamIo_e,
	Msg_Param_e,
	Msg_LogIo_e,
	Msg_GetUiCommands_e,
	Msg_RegUiCommand_e,
	Msg_FireUiCommand_e,
	Msg_UiCommandResult_e,
	
};
typedef enum messageTypes messageTypes_t;
}
#endif /* QUADGS_MESSAGE_TYPES_H_ */

