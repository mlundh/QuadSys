import argparse
import sys
import os
    
parser = argparse.ArgumentParser(description='Script to generate message types used in QuadGS. Messages have data carriers (members), signals do not. To create a signal, simply do not specify any members. ')
parser.add_argument('className', help='Name of the class')
parser.add_argument('--member', nargs=2, action='append', help='Member of the class. (type) (name)')
parser.add_argument('--include', nargs=1, action='append', help='Includes needed by the class.')


args = parser.parse_args()

directory = 'src/'
if not os.path.exists(directory):
    os.makedirs(directory)

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
		istream += 'is >> ' + deref + name + ';\n		'
		ostream += 'os << ' + deref + name + ';\n		'


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

with open(args.className + '.h', 'w') as f:
	f.write(templateStringHeader.format(**subst))
with open(directory+args.className + '.cpp', 'w') as f:
	f.write(templateStringImpl.format(**subst))