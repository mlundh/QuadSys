# Define an executable  
list(APPEND src_gs "gs_cli/src/main.cpp")
list(APPEND dep_gs ${Boost_LIBRARIES}
                    "serial_io" 
                    "cli" 
                    "Core"
                    "readline"
                    "pthread")
qgs_add_exe(QuadGS src_gs dep_gs)