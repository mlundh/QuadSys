set(src "LogHandler.cpp")
set(dep "CommandTree"
        "Messages"
        "QGS_Module")

qgs_make_shared_lib( LogHandler src dep)


#list(APPEND src_LogHandlerTest   "LogHandlerTest.cpp")
#list(APPEND dep_LogHandlerTest   "gtest_main"
#                                 "Parameters")

#qgs_add_test(LogHandlerTest src_LogHandlerTest dep_LogHandlerTest)