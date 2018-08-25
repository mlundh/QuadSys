list(APPEND src "Msg_FindParam.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( Ui_Command src dep)

