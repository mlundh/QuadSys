list(APPEND src "Msg_Param.cpp"
				)
                
list(APPEND dep "BinaryStream"
				"MsgBase")
				
qgs_make_shared_lib( GS_FC_Param src dep)

