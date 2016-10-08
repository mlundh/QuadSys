set(src "Parameters.cpp")
set(dep "CommandTree"
        "DataPackets"
        "CommandTree")

qgs_make_shared_lib( Parameters src dep)
