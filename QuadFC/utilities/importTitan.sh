#!/usr/bin/env bash
path=$1
if [[ -n "$path" ]] ; then
    rm -rf ../Drivers/Titan
    ./importCubeMX.sh $path ../Drivers/Titan
else
    echo "Requres source path."
fi

