list(APPEND src_testTop   "testCLIParam.cpp")
list(APPEND dep_testTop   "pthread"
                          "Parameters"
                          "cli"
                          "LogHandler"
                          "serial_io"
                          "dbgModule")

qgs_add_exe(testTop src_testTop dep_testTop)
