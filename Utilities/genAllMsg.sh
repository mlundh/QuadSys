#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pythonDIR="$DIR/pythonGenerator"
filename="$DIR/messages.txt"
addresses="$DIR/addresses.txt"
outputDIRcpp="$DIR/../QuadGS/Common/Messages"
outputDIRc="$DIR/../QuadFC/BaseComponents/Messages"

echo "Generating messages."
echo ""
python3 $pythonDIR/genMessage.py $filename $addresses $outputDIRcpp $outputDIRc
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error processing enum."
	exit $retVal
fi
