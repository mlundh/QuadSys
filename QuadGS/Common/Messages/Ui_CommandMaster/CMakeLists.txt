list(APPEND src "Msg_RegUiCommand.cpp"
				"Msg_UiCommandResult.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( Ui_CommandMaster src dep)

