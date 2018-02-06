add_subdirectory(LogHandler)
add_subdirectory(SerialIo)
add_subdirectory(Cli)
add_subdirectory(Parameters)

if(WITH_VISION)
    add_subdirectory(ZedTracker)
    #add_subdirectory(QGS_Viewer)
endif()
