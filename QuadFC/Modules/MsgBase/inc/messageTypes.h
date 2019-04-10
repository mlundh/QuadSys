#ifndef QUADFC_MESSAGE_TYPES_H_
#define QUADFC_MESSAGE_TYPES_H_

enum messageTypes
{
	Msg_NoType_e,
	Msg_Stop_e,
	Msg_RegisterName_e,
	Msg_Display_e,
	Msg_Test_e,
	Msg_FindParam_e,
	Msg_GetUiCommands_e,
	Msg_FireUiCommand_e,
	Msg_RegUiCommand_e,
	Msg_UiCommandResult_e,
	Msg_Param_e,
	Msg_ParamSave_e,
	Msg_ParamLoad_e,
	Msg_Log_e,
	Msg_Debug_e,
	Msg_Transmission_e,
	Msg_FlightMode_e,
	Msg_CtrlMode_e,
	Msg_FcFault_e,
	Msg_Error_e,
	Msg_FlightModeReq_e,
	Msg_CtrlModeReq_e,
	Msg_NewSetpoint_e,
	Msg_RegisterHandler_e,
	Msg_Subscriptions_e,
	Msg_FcLog_e,
	Msg_LogNameReq_e,
	LogStop_e,
	Msg_CtrlSig_e,
	Msg_NewState_e,
	Msg_HasParam_e,
	Msg_ParamFc_e,

	Msg_LastType_e,
};
typedef enum messageTypes messageTypes_t;

static const char* const messageTypesStr[] =
{
	"Msg_NoType",
	"Msg_Stop",
	"Msg_RegisterName",
	"Msg_Display",
	"Msg_Test",
	"Msg_FindParam",
	"Msg_GetUiCommands",
	"Msg_FireUiCommand",
	"Msg_RegUiCommand",
	"Msg_UiCommandResult",
	"Msg_Param",
	"Msg_ParamSave",
	"Msg_ParamLoad",
	"Msg_Log",
	"Msg_Debug",
	"Msg_Transmission",
	"Msg_FlightMode",
	"Msg_CtrlMode",
	"Msg_FcFault",
	"Msg_Error",
	"Msg_FlightModeReq",
	"Msg_CtrlModeReq",
	"Msg_NewSetpoint",
	"Msg_RegisterHandler",
	"Msg_Subscriptions",
	"Msg_FcLog",
	"Msg_LogNameReq",
	"LogStop",
	"Msg_CtrlSig",
	"Msg_NewState",
	"Msg_HasParam",
	"Msg_ParamFc",

	"Msg_LastType",
};

#endif /* QUADFC_MESSAGE_TYPES_H_ */

