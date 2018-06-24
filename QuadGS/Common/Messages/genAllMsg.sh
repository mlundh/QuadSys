#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pythonDIR="$DIR/../../Utilities/pythonGenerator"
filename="$DIR/messages.txt"

echo "Generating enum."
python $pythonDIR/genTypes.py $filename $DIR/../MsgBase
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error processing enum."
	exit $retVal
fi

echo "Generating cmake file."
python $pythonDIR/genCmake.py $filename $DIR/../Messages
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error processing cmake file."
	exit $retVal
fi

while read -r line
do
    name="$line"
    echo "Generating c++ - $name"
    python $pythonDIR/genMessage.py $name
    retVal=$?
	if [ $retVal -ne 0 ]; then
    	echo "Error processing $name"
    	exit $retVal
	fi
done < "$filename"
