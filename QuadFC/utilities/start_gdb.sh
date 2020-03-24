#!/usr/bin/env bash
gnome-terminal  --window-with-profile=StartEnv -e "JLinkGDBServer -device ATSAM3X8E -rtos GDBServer/RTOSPlugin_FreeRTOS"&
gdb-multiarch -command=./gdbScripts/gdbFC.init


