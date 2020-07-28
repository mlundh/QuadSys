#!/usr/bin/env bash
port=$1
if [[ -n "$port" ]]; then
	echo -ne "\033]0;Serial Port: ${port}\007"
	stty -F $port 115200 cs8 -cstopb -parenb crtscts 
	cat $port
else
    echo "Argument missing: serial port."
fi

