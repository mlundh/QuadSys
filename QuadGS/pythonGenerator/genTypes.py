import argparse
import sys
import os

parser = argparse.ArgumentParser(description='Script to generate an enum describing messages used in QuadGS.')

parser.add_argument('file', help='Name of the file containing all types.')
parser.add_argument('outputDir', help='output dir')

args = parser.parse_args()
types=''
dir = sys.path[0] + '/'

with open(args.file, "r") as file:
	array = []
	for line in file:
		types += (line.split(' ',1)[0]) + '_e,\n	'
subst={}
subst['types'] = types
with open(dir+'msgTypesTemplate', 'r') as ftemp:
	templateStringEnum = ftemp.read()
	

with open(args.outputDir+'/messageTypes.h', 'w') as f:
	f.write(templateStringEnum.format(**subst))