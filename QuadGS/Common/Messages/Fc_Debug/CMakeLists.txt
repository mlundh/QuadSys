list(APPEND src "Msg_Debug.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( Fc_Debug src dep)

