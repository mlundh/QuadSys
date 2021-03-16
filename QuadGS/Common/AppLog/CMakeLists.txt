list(APPEND src "AppLog.cpp")
list(APPEND dep "MsgBase"
                "GS_System")
qgs_make_shared_lib( AppLog src dep)

list(APPEND src_AppLogTest "AppLogTest.cpp")
list(APPEND dep_AppLogTest "gtest_main"
                            "AppLog")

qgs_add_test(AppLogTest src_AppLogTest dep_AppLogTest)