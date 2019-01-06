#!/usr/bin/env bash
port=$1
if [[ -n "$port" ]]; then
	stty -F $port 57600 cs8 -cstopb -parenb
	cat $port
else
    echo "argument error"
fi

