list(APPEND src "Msg_FindParam.cpp"
				"Msg_GetUiCommands.cpp"
				"Msg_FireUiCommand.cpp"
				"Msg_RegUiCommand.cpp"
				"Msg_UiCommandResult.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase")
				
qgs_make_shared_lib( GS_Ui_Command src dep)

