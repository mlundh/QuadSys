list(APPEND src "Msg_Param.cpp"
				"Msg_ParamSave.cpp"
				"Msg_ParamLoad.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase"
				"AppLog")
				
qgs_make_shared_lib( Param src dep)

