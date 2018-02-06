
list(APPEND src "ZedTracker.cpp" 
                "Viewer.cpp"
                "ZEDModel.cpp")

list(APPEND dep ${GLUT_LIBRARY}
                ${GLEW_LIBRARY}
                ${OPENGL_LIBRARIES})

qgs_make_shared_lib( ZedTracker src dep)