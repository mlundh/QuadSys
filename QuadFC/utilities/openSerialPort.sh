#!/usr/bin/env bash
port=$1
if [[ -n "$port" ]]; then
	stty -F $port 115200 cs8 -cstopb -parenb crtscts 
	cat $port
else
    echo "Argument missing: serial port."
fi

