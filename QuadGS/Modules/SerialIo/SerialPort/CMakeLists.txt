list(APPEND src "SerialPort.cpp")
list(APPEND dep "Messages" 
                "AppLog"
                "SlipPacket"
                "Fifo")

qgs_make_shared_lib( SerialPort src dep)
