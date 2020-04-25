#!/usr/bin/env bash

target=$2
path=$1
if [[ -n "$path" ]] && [[ -n "$target" ]] ; then

    cp -r $path $target
    find $target -type f -name "*.h" | xargs sed -i "s/#include \"main.h\"/#include \"cubeInit.h\"/g"
    find $target -type f -name "*.c" | xargs sed -i "s/#include \"main.h\"/#include \"cubeInit.h\"/g"
    find $target -type f -name "Makefile" | xargs sed -i "s/main.h/cubeInit.h/g"
    find $target -type f -name "Makefile" | xargs sed -i "s/main.c/cubeInit.c/g"

    mv $target/Core/Inc/main.h $target/Core/Inc/cubeInit.h
    mv $target/Core/Src/main.c $target/Core/Src/cubeInit.c

else
    echo "Requres both source and target."
fi

