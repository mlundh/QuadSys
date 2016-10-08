add_subdirectory(SerialPort)
add_subdirectory(SlipPacket)
add_subdirectory(crc)


list(APPEND src "SerialManager.cpp" )

list(APPEND dep "SerialPort"
                "SlipPacket"
                "crc"
                "log"
                "Parameters")

qgs_make_shared_lib( serial_io src dep)
