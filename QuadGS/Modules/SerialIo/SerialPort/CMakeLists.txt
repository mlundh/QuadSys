list(APPEND src "SerialPort.cpp")
list(APPEND dep "MsgBase" 
                "AppLog"
                "SlipPacket"
                "Fifo"
                "Parser")

qgs_make_shared_lib( SerialPort src dep)
