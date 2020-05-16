#!/usr/bin/env bash
path=$1
target="../../Drivers/Titan"
if [[ -n "$path" ]] ; then
    rm -rf $target
    ./importCubeMX.sh $path $target
    ./cubeMXoverride.sh cubeMXoverride.txt $target

else
    echo "Requres source path."
fi

