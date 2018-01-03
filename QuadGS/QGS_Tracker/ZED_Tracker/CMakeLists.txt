
list(APPEND src "ZedTracker.cpp" )

list(APPEND dep "Messages"
                ${ZED_LIBRARIES}
                "Core")

qgs_make_shared_lib( ZedTracker src dep)