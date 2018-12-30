list(APPEND src_testTop   "testCLIParam.cpp")
list(APPEND dep_testTop   "Parameters"
                          "cli"
                          "LogHandler"
                          "serial_io"
                          "dbgModule")

qgs_add_test(testTop src_testTop dep_testTop)