#!/usr/bin/env bash
if [[ $PLATFORM == "Titan" ]];
then
   device='STM32F413VG'
elif [[ "$PLATFORM" == "Due" ]]
then
   device='ATSAM3X8E'
else
   echo "Unsupported platform."
   exit 1
fi

gnome-terminal  --window-with-profile=StartEnv -e "JLinkGDBServer -device $device -if SWD -rtos GDBServer/RTOSPlugin_FreeRTOS"&
cd gdbScripts/$PLATFORM
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
echo $DIR
gdb-multiarch -command=./gdbFC.init


