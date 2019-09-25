import argparse
import sys
import os
from enum import Enum
from distutils.dir_util import copy_tree
import shutil

def genCppMessage( lineIn, outputDirCpp):
	parser = argparse.ArgumentParser(description='Script to generate message types used in QuadGS. Messages have data carriers (members), signals do not. To create a signal, simply do not specify any members. ')
	parser.add_argument('className', help='Name of the class')
	parser.add_argument('--member', nargs=2, action='append', help='Member of the class. (type) (name)')
	parser.add_argument('--include', nargs=1, action='append', help='Includes needed by the class.')
	
	
	args = parser.parse_args(lineIn.split())
	
	baseDirectory = outputDirCpp
	if not os.path.exists(baseDirectory):
		os.makedirs(baseDirectory)
	
	srcDirectory = outputDirCpp+'/src'
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
				with open(dir+'/c++/memberCloneTemplateHeader', 'r') as ftemp:
					templateString = ftemp.read()
					mFcnHeader += templateString.format(**mSubst)
				with open(dir+'/c++/memberCloneTemplateImpl', 'r') as ftemp:
					templateString = ftemp.read()
					mFcnImpl += templateString.format(**mSubst)
	
			mSubst['nameinput'] = nameinput
			mSubst['move'] = move
			mSubst['moveEnd'] = moveEnd
			with open(dir+'/c++/memberFcnTemplateHeader', 'r') as ftemp:
				templateString = ftemp.read()
				mFcnHeader += templateString.format(**mSubst)
			with open(dir+'/c++/memberFcnTemplateImpl', 'r') as ftemp:
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
	
	with open(dir+'/c++/classTemplateHeader', 'r') as ftemp:
		templateStringHeader = ftemp.read()
		
	with open(dir+'/c++/classTemplateImpl', 'r') as ftemp:
		templateStringImpl = ftemp.read()
	
	with open(baseDirectory + '/' + args.className + '.h', 'w') as f:
		f.write(templateStringHeader.format(**subst))
	with open(srcDirectory + '/' +args.className + '.cpp', 'w') as f:
		f.write(templateStringImpl.format(**subst))
	return



def genCMessage( lineIn, outputDirC):
	parser = argparse.ArgumentParser(description='Script to generate message types used in QuadGS. Messages have data carriers (members), signals do not. To create a signal, simply do not specify any members. ')
	parser.add_argument('className', help='Name of the class')
	parser.add_argument('--member', nargs=2, action='append', help='Member of the class. (type) (name)')
	parser.add_argument('--include', nargs=1, action='append', help='Includes needed by the class.')
	parser.add_argument('--internal', action = 'store_true')
	
	args = parser.parse_args(lineIn.split())
	
	baseDirectory = outputDirC
	if not os.path.exists(baseDirectory):
		os.makedirs(baseDirectory)
	
	srcDirectory = outputDirC+'/src'
	if not os.path.exists(srcDirectory):
	    os.makedirs(srcDirectory)
	
	incDirectory = outputDirC+'/inc'
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
	createArgs = ''
	createCpyInit  = ''
	createArgsInit = ''
	serialize = ''
	deserialize = ''
	stringInit = ''
	stringInitDeserialize = ''
	index = 0;

	class StringStatus(Enum):
		notString = 1
		String = 2
		PayloadLength = 3
		BufferLength = 4
	if args.member:
		for value in args.member:
			value.append(StringStatus.notString) # add an "is string" to the list 

	if args.member:
		for value in args.member:
			type = value[0]
			name = 'm' + value[1].title()
			nameinput = value[1]
			nameFcn = value[1].title()
			cpyname = name
			serializationType = type
			if (type.find('std::string') != -1): # handle the string case.
				type = 'uint8_t*'#translate to c string.
				value[2] = StringStatus.String # this is a string
				serializationType = 'string'
				args.member.append(['uint32_t',nameFcn+'length', StringStatus.PayloadLength]) #add the Length parameter.
				args.member.append(['uint32_t',nameFcn+'bufferlength', StringStatus.BufferLength]) #add the bufferLength parameter. This should be considered not a string sinde it will be a part of the interface.
				stringInit += ' + (' + nameFcn+'bufferlength)'
				stringInitDeserialize += ' + (buffer_size)'
			mSubst = {}
			mSubst['msgName'] = args.className
			mSubst['nameFcn'] = nameFcn
			mSubst['type'] = type
			mSubst['serializationType'] = serializationType
			mSubst['name'] = name
			mSubst['nameinput'] = nameinput
			with open(dir+'/c/fcnHeaderTemplate', 'r') as ftemp:
				templateString = ftemp.read()
				mFcnHeader += templateString.format(**mSubst)
			with open(dir+'/c/fcnImplTemplate', 'r') as ftemp:
				templateString = ftemp.read()
				mFcnImpl += templateString.format(**mSubst)
			if(not args.internal): # Slip serialization of internal messages not meant for sending to other nodes. 
				if(value[2] == StringStatus.String): #this is a string.
					with open(dir+'/c/serializeStringTemplate', 'r') as ftemp:
						templateString = ftemp.read()
						serialize += templateString.format(**mSubst)
					with open(dir+'/c/deSerializeStringTemplate', 'r') as ftemp:
						templateString = ftemp.read()
						deserialize += templateString.format(**mSubst)
				elif(value[2] == StringStatus.notString):
					with open(dir+'/c/serializeTypeTemplate', 'r') as ftemp:
						templateString = ftemp.read()
						serialize += templateString.format(**mSubst)
					with open(dir+'/c/deSerializeTypeTemplate', 'r') as ftemp:
						templateString = ftemp.read()
						deserialize += templateString.format(**mSubst)
					
			mDecl += type + ' ' + name + ';\n	'
			if(value[2] == StringStatus.notString or value[2] == StringStatus.BufferLength): #this is not a string
				createArgs += ', ' + type + ' ' + value[1]
				createArgsInit +=  '        ' + 'internal_data->'+name + ' = ' + nameinput + ';\n'
			elif(value[2] == StringStatus.PayloadLength):
				createArgsInit +=  '        ' + 'internal_data->'+name + ' = ' + '0;\n'
			elif(value[2] == StringStatus.String):
				createArgsInit +=  '        ' + 'internal_data->'+name + ' = ' + '(uint8_t*)(internal_data+1);\n'

			index+=1

	subst = {}
	subst['stringInit'] = stringInit
	subst['stringInitDeserialize'] = stringInitDeserialize
	subst['includes'] = includes
	subst['msgName'] = args.className
	subst['msgNameUpper'] = args.className.upper()
	subst['createArgs'] = createArgs
	subst['createArgsInit'] = createArgsInit
	subst['members'] = mDecl
	subst['memberFcnHeader'] = mFcnHeader
	subst['memberFcnImpl'] = mFcnImpl
	subst['serialize'] = serialize
	subst['deserialize'] = deserialize
	with open(dir+'/c/msgTemplateHeader', 'r') as ftemp:
		templateStringHeader = ftemp.read()
		
	with open(dir+'/c/msgTemplateImpl', 'r') as ftemp:
		templateStringImpl = ftemp.read()
	
	with open(incDirectory + '/' + args.className + '.h', 'w') as f:
		f.write(templateStringHeader.format(**subst))
	with open(srcDirectory + '/' +args.className + '.c', 'w') as f:
		f.write(templateStringImpl.format(**subst))
	return
def genCmake( files, interface, outputDirCpp ):
	dir = sys.path[0] + '/'
	substCmake={}
	substCmake['interface'] = interface
	substCmake['files'] = files
	with open(dir+'/c++/cmakeTemplate', 'r') as ftemp:
		templateStringCmake = ftemp.read()
	with open(outputDirCpp+'/CMakeLists.txt', 'w') as f:
		f.write(templateStringCmake.format(**substCmake))

def genTypes( types, enumName, outputDirCpp, outputDirC):
	dir = sys.path[0] + '/'
	enum = ''
	enumStr = ''
	substTypes={}
	for type in types:
		enum += '	' + type + '_e,\n'
		enumStr += '	"' + type + '",\n'
		#print('types: ' + type)

	substTypes['enumName'] = enumName
	substTypes['enum'] = enum
	substTypes['enumStr'] = enumStr
	with open(dir+'/c++/msgTypesTemplate', 'r') as ftemp:
		templateStringEnum = ftemp.read()
		
	with open(outputDirCpp+'/messageTypes.h', 'w') as f:
		f.write(templateStringEnum.format(**substTypes))
		
	with open(dir+'/c/msgTypesTemplate', 'r') as ftemp:
		templateStringEnum = ftemp.read()
		
	with open(outputDirC+'/inc/messageTypes.h', 'w') as f:
		f.write(templateStringEnum.format(**substTypes))

def genParsercpp( types, interfaces, outputDirCpp ):
	dir = sys.path[0] + '/'
	typeParser = ""
	headers = ""
	for type in types:
		headers += '#include "' + type + '.h"\n'
		substTypes={}
		substTypes['type'] = type
		with open(dir+'/c++/messageTypeParserTemplate', 'r') as ftemp:
			templateString = ftemp.read()
			typeParser += templateString.format(**substTypes)
	substTypes = {}
	substTypes['headers'] = headers
	substTypes['typeParser']=typeParser
	with open(dir+'/c++/parserTemplate', 'r') as ftemp:
		templateStringParser = ftemp.read()
	
	if not os.path.exists(outputDirCpp+'/src/'):
		os.makedirs(outputDirCpp+'/src/')
		
	with open(outputDirCpp+'/src/Parser.cpp', 'w') as f:
		f.write(templateStringParser.format(**substTypes))
		
	deps=''
	for interface in interfaces:
		deps += interface + '\n				'
	subst = {}
	subst['dep'] = deps
	with open(dir+'/c++/parserCmakeTemplate', 'r') as ftemp:
		templateStringCmake = ftemp.read()
		

	with open(outputDirCpp+'/CMakeLists.txt', 'w') as f:
		f.write(templateStringCmake.format(**subst))
	
	copy_tree(dir+'/c++/parser/', outputDirCpp)
	
def genParserc( types, interfaces, outputDirCpp ):
	dir = sys.path[0] + '/'
	typeParser = ""
	typeSerializer = ""
	headers = ""
	for type in types:
		headers += '#include "../../Messages/inc/' + type + '.h"\n'
		substTypes={}
		substTypes['type'] = type
		with open(dir+'/c/messageTypeParserTemplate', 'r') as ftemp:
			templateString = ftemp.read()
			typeParser += templateString.format(**substTypes)
		with open(dir+'/c/messageTypeSerializerTemplate', 'r') as ftemp:
			templateString = ftemp.read()
			typeSerializer += templateString.format(**substTypes)
	substTypes = {}
	substTypes['headers'] = headers
	substTypes['typeParser']=typeParser
	substTypes['typeSerializer']=typeSerializer

	with open(dir+'/c/parserTemplate', 'r') as ftemp:
		templateStringParser = ftemp.read()
		
	with open(outputDirCpp+'/src/Msg_Parser.c', 'w') as f:
		f.write(templateStringParser.format(**substTypes))
		
	shutil.copy2(dir+'/c/Msg_Parser.h', outputDirCpp + '/inc/')



def genAdresses(addresses, enumName, outputDirCpp, outputDirC):
	
	regionsInt = 0
	regionStrings = ''
	masks = []
	region = ''
	currentRegion = 0
	currentAddress = 0
	addressString = ''
	enumString = ''
	#Go through all interfaces and generate messages, enums and addresses based on that. 
	with open(args.addresses, "r") as file:
		for line in file:
			if (line[0] == '*'): # Interface divider.
				regionTest = line.split()[0][1:]
				if (len(regionTest) > 1):
					region = regionTest
					regionint = (  currentRegion << 8)
					currentAddress = 0
					regionStrings += "	" + region + "_e = {},\n".format(hex(regionint))
				else:
					currentRegion += 1
				continue
			if (line == '\n' or line == '' or line[0] == '#'): #Skipp newline, empty line and comments.
				continue
			address = (currentRegion << 8) + currentAddress;
			currentAddress += 1
			addressString += "	" + line.rstrip() + "_e = {},\n".format(hex(address))
			enumString += "	{" + hex(address) + ", " + '"' + line.rstrip() + '_e"' + "},\n"
	

	substTypes = {}
	substTypes['regionStrings'] = regionStrings
	substTypes['enumNameRegion'] = enumName+"Domain"
	substTypes['enumName'] = enumName
	substTypes['addresses'] = addressString
	substTypes['strings'] = enumString
	with open(dir+'/c++/msgAddressesTemplate', 'r') as ftemp:
		templateStringEnum = ftemp.read()
		
	with open(outputDirCpp+'/msgAddr.h', 'w') as f:
		f.write(templateStringEnum.format(**substTypes))

	with open(dir+'/c/msgAddressesTemplate', 'r') as ftemp:
		templateStringEnum = ftemp.read()
		
	with open(outputDirC+'/inc/msgAddr.h', 'w') as f:
		f.write(templateStringEnum.format(**substTypes))
		
		
def genMsgEnums(outputDirCpp, outputDirC):

	with open(dir+'msg_enums', 'r') as ftemp:
		msg_enums = ftemp.read()
		
	substTypes = {}
	substTypes['msg_enums'] = msg_enums
	
	with open(dir+'/c++/msgEnumsTemplate', 'r') as ftemp:
		templateStringEnumTypes = ftemp.read()
	with open(outputDirCpp+'/msg_enums.h', 'w') as f:
		f.write(templateStringEnumTypes.format(**substTypes))
		
	with open(dir+'/c/msgEnumsTemplate', 'r') as ftemp:
		templateStringEnumTypes = ftemp.read()
	with open(outputDirC+'/inc/msg_enums.h', 'w') as f:
		f.write(templateStringEnumTypes.format(**substTypes))

def genCommonTypes(outputDirCpp, outputDirC):

	with open(dir+'common_types', 'r') as ftemp:
		common_types = ftemp.read()
		
	stream = ''
	with open(dir+'/common_types_stream', 'r') as ftemp:
		stream = ftemp.read()
	substTypes = {}
	substTypes['common_types'] = common_types
	substTypes['stream'] = stream
	
	with open(dir+'/c++/common_types', 'r') as ftemp:
		commonTypesTemplate = ftemp.read()
	with open(outputDirCpp+'/common_types.h', 'w') as f:
		f.write(commonTypesTemplate.format(**substTypes))
		
	with open(dir+'/c/common_types', 'r') as ftemp:
		commonTypesTemplate = ftemp.read()
	with open(outputDirC+'/inc/common_types.h', 'w') as f:
		f.write(commonTypesTemplate.format(**substTypes))

# Generate enums and messages.
parser = argparse.ArgumentParser(description='Script to generate a cmake file to build the message class.')

parser.add_argument('file', help='Name of the file containing all types.')
parser.add_argument('addresses', help='Name of the file containing all addresses.')
parser.add_argument('outputDirCpp', help='output dir cpp code')
parser.add_argument('outputDirC', help='output dir c code')

args = parser.parse_args()

interfaces = []
interface = ''
usedByGS = False
usedByFC = False
interfaceUsedByGS = []
dir = sys.path[0] + '/'
cTypes = []
cppTypes = []
cTypesSerializable = []
messageCmake = ''

#Go through all interfaces and generate messages, enums and addresses based on that. 
with open(args.file, "r") as file:
	files=''
	for line in file:
		if (line[0] == '*'): # Interface divider.
			usedByGS = False
			usedByFC = False
			interfaceTest = line.split()[0][1:]
			if (len(interfaceTest) > 1):
				interface = interfaceTest
				interfaces.append(interface)
				if(interface.find('GS_') != -1):
					usedByGS = True
				if(interface.find('FC_') != -1):
					usedByFC = True
				if(usedByGS):
					messageCmake += 'add_subdirectory(' + interface + ')\n' 
					interfaceUsedByGS.append(interface)
			else:
				genCmake(files, interface, args.outputDirCpp+'/'+interface)
				files = ''
			continue
		if (line == '\n' or line == '' or line[0] == '#'): #Skipp newline, empty line and comments.
			continue
		print('Generating: ' + line)
		if(usedByGS): # TODO Improve the tag so this is scalable...
			genCppMessage(line, args.outputDirCpp+'/'+interface)
			cppTypes.append(line.split()[0])
		if(usedByFC):
			genCMessage(line, args.outputDirC+'/')
			cTypesSerializable.append(line.split()[0])
		cTypes.append(line.split()[0])
		files += '"' + (line.split()[0]) + '.cpp"\n				'

genTypes(cTypes, 'messageTypes', args.outputDirCpp, args.outputDirC)
genMsgEnums(args.outputDirCpp, args.outputDirC)
genAdresses(args.addresses, 'msgAddr', args.outputDirCpp, args.outputDirC)
genCommonTypes(args.outputDirCpp , args.outputDirC)
genParsercpp(cppTypes, interfaceUsedByGS, args.outputDirCpp + '/Parser')
genParserc(cTypesSerializable, interfaceUsedByGS, args.outputDirC)

# Generate the top level Message cmakelist.
messageCmake += "add_subdirectory(Parser)\n" # Parser is a special case.
substTypes = {}
substTypes['cmakeTemplate'] = messageCmake
with open(dir+'/c++/messageCmakeTemplate', 'r') as ftemp:
	cmakeTemplate = ftemp.read()
	
with open(args.outputDirCpp+'/CMakeLists.txt', 'w') as f:
	f.write(cmakeTemplate.format(**substTypes))
