list(APPEND src "Parser.cpp" )

list(APPEND dep GS_System
				GS_Ui_Command
				GS_FC_Param
				GS_FC_Log
				GS_FC_Debug
				GS_FC_IO
				GS_FC_Status
				GS_FC_Control
				)

qgs_make_shared_lib( Parser src dep)

list(APPEND src_Test "ParserTest.cpp")
                     
list(APPEND dep_Test "gtest_main"
                     "QGS_Module"
                     "Parser")

qgs_add_test(ParserTest src_Test dep_Test)