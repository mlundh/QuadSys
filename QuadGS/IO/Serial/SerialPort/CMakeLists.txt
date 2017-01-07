list(APPEND src "SerialPort.cpp")
list(APPEND dep "DataMsg" 
                "log"
                "SlipPacket")

qgs_make_shared_lib( SerialPort src dep)
