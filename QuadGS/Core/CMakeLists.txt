add_subdirectory(CommandTree)
add_subdirectory(BinaryStream)
add_subdirectory(DataMsg)
add_subdirectory(Parameters)
add_subdirectory(Log)


set(src Core.cpp)

set(dep "CommandTree"
        "DataMsg"
        "Parameters" )

qgs_make_shared_lib( Core src dep)


set(src_MockCore MockCore.cpp)
set(dep_MockCore "gtest"
				 "gmock"
			 	 "Core" )
qgs_make_mock_shared_lib( MockCore src_MockCore dep_MockCore)
