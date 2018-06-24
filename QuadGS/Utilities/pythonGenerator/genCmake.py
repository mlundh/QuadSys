import argparse
import sys
import os

parser = argparse.ArgumentParser(description='Script to generate a cmake file to build the message class.')

parser.add_argument('file', help='Name of the file containing all types.')
parser.add_argument('outputDir', help='output dir')

args = parser.parse_args()
files=''
dir = sys.path[0] + '/'

with open(args.file, "r") as file:
	array = []
	for line in file:
		files += '"' + (line.split()[0]) + '.cpp"\n				'
subst={}
subst['files'] = files
with open(dir+'cmakeTemplate', 'r') as ftemp:
	templateStringCmake = ftemp.read()
	

with open(args.outputDir+'/CMakeLists.txt', 'w') as f:
	f.write(templateStringCmake.format(**subst))