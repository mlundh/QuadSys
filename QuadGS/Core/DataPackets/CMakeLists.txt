list(APPEND src "QspPayloadRaw.cpp"
                "QuadParamPacket.cpp"
                "QuadSerialPacket.cpp")
qgs_make_shared_lib( DataPackets src "")
