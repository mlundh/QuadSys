list(APPEND src "CLI.cpp")
list(APPEND dep "AppLog"
                "Parameters"
                "Core"
		        "readline")
qgs_make_shared_lib( cli src dep)
