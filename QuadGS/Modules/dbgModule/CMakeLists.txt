set(src "dbgModule.cpp")
set(dep "CommandTree"
        "GS_FC_Param"
        "GS_Ui_Command"
        "QGS_Module"
        "GS_FC_Debug")

qgs_make_shared_lib( dbgModule src dep)


list(APPEND src_dbgModuleTest   "dbgModuleTest.cpp")
list(APPEND dep_dbgModuleTest   "gtest_main"
                                "dbgModule")

qgs_add_test(dbgModuleTest src_dbgModuleTest dep_dbgModuleTest)