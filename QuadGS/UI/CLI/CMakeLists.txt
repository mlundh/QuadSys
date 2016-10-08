list(APPEND src "QuadCLI.cpp")
list(APPEND dep "log"
                "Parameters"
                "Core")
qgs_make_shared_lib( cli src dep)
