set(src "LogHandler.cpp")
set(dep "CommandTree"
        "Ui_Command"
        "Ui_CommandSlave"
        "Ui_CommandMaster"
        "Log"
        "QGS_Module")

qgs_make_shared_lib( LogHandler src dep)


list(APPEND src_Test   "LogHandlerTest.cpp")
list(APPEND dep_Test   "gtest_main"
                       "LogHandler"
                       "TestHelpers"
                       "MsgBase")

qgs_add_test(LogHandlerTest src_Test dep_Test)