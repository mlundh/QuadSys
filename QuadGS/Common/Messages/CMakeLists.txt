list(APPEND src "Msg_Stop.cpp"
				"Msg_DebugIo.cpp"
				"Msg_ParamIo.cpp"
				"Msg_Param.cpp"
				"Msg_LogIo.cpp"
				"Msg_GetUiCommands.cpp"
				"Msg_RegUiCommand.cpp"
				"Msg_FireUiCommand.cpp"
				"Msg_UiCommandResult.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( Messages src dep)

