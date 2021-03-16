# Define an executable  
list(APPEND src_gs "src/main.cpp")
list(APPEND dep_gs ${Boost_LIBRARIES}
                    "pthread"
                    "Parameters"
                    "cli"
                    "LogHandler"
                    "serial_io"
                    "dbgModule"
                    "AppLogHandler")
if(WITH_VISION)
#    list(APPEND dep_gs "ZedTracker"
#                       )
endif()
qgs_add_exe(QuadGS src_gs dep_gs)

add_subdirectory(test)
