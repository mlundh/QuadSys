list(APPEND src "Parser.cpp" )

list(APPEND dep GS_System
				Param
				Log
				Ui_Command
				IO
				)

qgs_make_shared_lib( Parser src dep)

list(APPEND src_Test "ParserTest.cpp")
                     
list(APPEND dep_Test "gtest_main"
                     "QGS_Module"
                     "Parser")

qgs_add_test(ParserTest src_Test dep_Test)