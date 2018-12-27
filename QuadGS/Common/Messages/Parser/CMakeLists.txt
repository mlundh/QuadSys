list(APPEND src "Parser.cpp" )

list(APPEND dep "BinaryStream"
                "MsgBase"
                "Param")

qgs_make_shared_lib( Parser src dep)

list(APPEND src_Test "ParserTest.cpp")
                     
list(APPEND dep_Test "gtest_main"
                     "QGS_Module"
                     "Param"
                     "Parser")

qgs_add_test(Parser_test src_Test dep_Test)