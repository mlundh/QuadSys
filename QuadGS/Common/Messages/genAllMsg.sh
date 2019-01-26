#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pythonDIR="$DIR/../../../Utilities/pythonGenerator"
filename="$DIR/messages.txt"

echo "Generating messages."
echo ""
python $pythonDIR/genMessage.py $filename $DIR
retVal=$?
if [ $retVal -ne 0 ]; then
	echo "Error processing enum."
	exit $retVal
fi
