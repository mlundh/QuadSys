list(APPEND src "Semaphore.cpp")
                
list(APPEND dep "")

qgs_make_shared_lib( Semaphore src dep)

