#!/usr/bin/env bash
port=$1
speed=$2
if [[ -n "$port" ]]; then
	if [[ -z "$speed" ]]; then
		speed=460800
	fi
	echo -ne "\033]0;Serial Port: ${port}\007"
	stty -F $port $speed cs8 -cstopb -parenb crtscts 
	cat $port
else
    echo "Argument missing: serial port."
fi

