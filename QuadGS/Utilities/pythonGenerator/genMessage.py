import argparse
import sys
import os



def genMessage( lineIn, outputDir ):
	parser = argparse.ArgumentParser(description='Script to generate message types used in QuadGS. Messages have data carriers (members), signals do not. To create a signal, simply do not specify any members. ')
	parser.add_argument('className', help='Name of the class')
	parser.add_argument('--member', nargs=2, action='append', help='Member of the class. (type) (name)')
	parser.add_argument('--include', nargs=1, action='append', help='Includes needed by the class.')
	
	
	args = parser.parse_args(lineIn.split())
	
	baseDirectory = outputDir
	if not os.path.exists(baseDirectory):
		os.makedirs(baseDirectory)
	
	srcDirectory = outputDir+'/src'
	if not os.path.exists(srcDirectory):
	    os.makedirs(srcDirectory)
	
	includes = ''
	if args.include:
		for value in args.include:
			includes += '#include "' + value[0] + '"\n'
	
	dir = sys.path[0] + '/'
	mFcnHeader = ''
	mFcnImpl = ''
	mDecl = ''
	ctorArgs = ''
	ctorInit = ''
	istream = ''
	ostream = ''
	ctorCpyInit  = ''
	
	if args.member:
		for value in args.member:
			
			type = value[0]
			coreType = ''
	
			move = ''
			moveEnd = ''
			const = 'const'
			deref = ''
			name = 'm' + value[1].title()
			nameinput = value[1]
			nameFcn = value[1].title()
			cpyname = name
			mSubst = {}
			mSubst['className'] = args.className
			mSubst['nameFcn'] = nameFcn
			mSubst['type'] = type
			mSubst['name'] = name
			mSubst['const'] = const
			#special case for unique_ptr
			if (type.find('std::unique_ptr') != -1): 
				move = 'std::move('
				moveEnd = ')'
				cpyname = 'getClone'+nameFcn+'()'
				const = ''
				deref = '*'
				pos1 = type.find('<') # remove std::unique_ptr to get core type.
				coreType = type[(pos1+1):] 
				coreType = coreType[:-1] 
				mSubst['coreType'] = coreType
				mSubst['const'] = const
				with open(dir+'memberCloneTemplateHeader', 'r') as ftemp:
					templateString = ftemp.read()
					mFcnHeader += templateString.format(**mSubst)
				with open(dir+'memberCloneTemplateImpl', 'r') as ftemp:
					templateString = ftemp.read()
					mFcnImpl += templateString.format(**mSubst)
	
			mSubst['nameinput'] = nameinput
			mSubst['move'] = move
			mSubst['moveEnd'] = moveEnd
			with open(dir+'memberFcnTemplateHeader', 'r') as ftemp:
				templateString = ftemp.read()
				mFcnHeader += templateString.format(**mSubst)
			with open(dir+'memberFcnTemplateImpl', 'r') as ftemp:
				templateString = ftemp.read()
				mFcnImpl += templateString.format(**mSubst)
				
			mDecl += type + ' ' + name + ';\n	'
			ctorArgs += ', ' + type + ' ' + value[1] 
			ctorInit += ', ' + name + '(' + move + value[1] + moveEnd + ')'
			ctorCpyInit += ', ' + name + '(' + move + 'msg.' + cpyname +  moveEnd + ')'
			if(type.find('std::string') != -1):
				istream += name+'.erase();\n	'
			istream += 'is >> ' + deref + name + ';\n	'
			ostream += 'os << ' + deref + name + ';\n	'
	
	
	subst = {}
	subst['includes'] = includes
	subst['className'] = args.className
	subst['classNameUpper'] = args.className.upper()
	subst['ctorArgs'] = ctorArgs
	subst['ctorInit'] = ctorInit
	subst['ctorCpyInit'] = ctorCpyInit
	subst['members'] = mDecl
	subst['memberFcnHeader'] = mFcnHeader
	subst['memberFcnImpl'] = mFcnImpl
	subst['istream'] = istream
	subst['ostream'] =  ostream
	
	with open(dir+'classTemplateHeader', 'r') as ftemp:
		templateStringHeader = ftemp.read()
		
	with open(dir+'classTemplateImpl', 'r') as ftemp:
		templateStringImpl = ftemp.read()
	
	with open(baseDirectory + '/' + args.className + '.h', 'w') as f:
		f.write(templateStringHeader.format(**subst))
	with open(srcDirectory + '/' +args.className + '.cpp', 'w') as f:
		f.write(templateStringImpl.format(**subst))
	return

def genCmake( files, interface, outputDir ):
	dir = sys.path[0] + '/'
	substCmake={}
	substCmake['interface'] = interface
	substCmake['files'] = files
	with open(dir+'cmakeTemplate', 'r') as ftemp:
		templateStringCmake = ftemp.read()
	with open(outputDir+'/CMakeLists.txt', 'w') as f:
		f.write(templateStringCmake.format(**substCmake))

def genTypes( types, enumName, outputDir ):
	dir = sys.path[0] + '/'
	substTypes={}
	substTypes['enumName'] = enumName
	substTypes['types'] = types
	with open(dir+'msgTypesTemplate', 'r') as ftemp:
		templateStringEnum = ftemp.read()
		
	with open(outputDir+'/messageTypes.h', 'w') as f:
		f.write(templateStringEnum.format(**substTypes))


# Generate enums and messages.
parser = argparse.ArgumentParser(description='Script to generate a cmake file to build the message class.')

parser.add_argument('file', help='Name of the file containing all types.')
parser.add_argument('outputDir', help='output dir')

args = parser.parse_args()

interfaces = []
interface = ''
dir = sys.path[0] + '/'
with open(args.file, "r") as file:
	files=''
	types=''
	for line in file:
		if (line[0] == '*'):
			interfaceTest = line.split()[0][1:]
			if (len(interfaceTest) > 1):
				interface = interfaceTest
				interfaces.append(interface)
			else:
				genCmake(files, interface, args.outputDir+'/'+interface)
				files = ''
			continue
		if (line == '\n' or line == '' or line[0] == '#'):
			continue
		print('Generating: ' + line)
		genMessage(line, args.outputDir+'/'+interface)
		types += (line.split()[0]) + '_e,\n	'
		files += '"' + (line.split()[0]) + '.cpp"\n				'
genTypes(types, 'messageTypes', args.outputDir + '/../MsgBase')

messageCmake = ''
for member in interfaces:
	messageCmake += 'add_subdirectory(' + member + ')\n' 
with open(args.outputDir+'/CMakeLists.txt', 'w') as f:
	f.write(messageCmake)
