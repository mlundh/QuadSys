list(APPEND src "Msg_Log.cpp"
				"Msg_AppLog.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase")
				
qgs_make_shared_lib( GS_FC_Log src dep)

