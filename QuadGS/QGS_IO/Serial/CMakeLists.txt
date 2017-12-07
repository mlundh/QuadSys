add_subdirectory(SerialPort)
add_subdirectory(SlipPacket)
add_subdirectory(crc)
add_subdirectory(FcParser)


list(APPEND src "SerialManager.cpp" )

list(APPEND dep "SerialPort"
                "SlipPacket"
                "crc"
                "log"
                "Parameters"
                "FcParser")

qgs_make_shared_lib( serial_io src dep)
