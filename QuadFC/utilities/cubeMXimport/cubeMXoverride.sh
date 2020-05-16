#!/usr/bin/env bash
replacefile=$1
path=$2
onlyOnce="__attribute__((weak))"

if [[ -n "$replacefile" ]] && [[ -n "$path" ]] ; then
    while IFS= read -r line
    do
        find $path -type f -name "*.h" | xargs sed -i  " /$onlyOnce/! s/$line/$onlyOnce $line/g"
    done < "$replacefile"
else 
    echo "no input file"
fi
