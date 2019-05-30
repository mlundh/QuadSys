#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pythonDIR="$DIR/pythonGenerator"
filename="$DIR/messages.txt"
outputDIRcpp="$DIR/../QuadGS/Common/Messages"
outputDIRc="$DIR/../QuadFC/Modules/Messages"

echo "Generating messages."
echo ""
python3 $pythonDIR/genMessage.py $filename $outputDIRcpp $outputDIRc
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error processing enum."
	exit $retVal
fi
