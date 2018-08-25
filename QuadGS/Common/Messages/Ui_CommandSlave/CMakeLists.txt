list(APPEND src "Msg_GetUiCommands.cpp"
				"Msg_FireUiCommand.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( Ui_CommandSlave src dep)

