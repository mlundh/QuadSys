list(APPEND src "SlipPacket.cpp")
list(APPEND dep "CommandTree"
                "DataPackets"
                "crc")
qgs_make_shared_lib( SlipPacket src dep)
