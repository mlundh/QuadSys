add_subdirectory(CommandTree)
add_subdirectory(DataPackets)
add_subdirectory(Parameters)


set(src Core.cpp)

set(dep "CommandTree"
        "DataPackets"
        "Parameters" )

qgs_make_shared_lib( Core src dep)
