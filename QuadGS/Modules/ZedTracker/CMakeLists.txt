
list(APPEND src "ZedTracker.cpp" )

list(APPEND dep ${ZED_LIBRARIES}
                "Ui_Command"
                "QGS_Module")

qgs_make_shared_lib( ZedTracker src dep)

if(CMAKE_COMPILER_IS_GNUCXX)
	target_compile_options(ZedTracker PRIVATE -Wno-deprecated-declarations )
endif()