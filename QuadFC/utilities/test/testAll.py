import os
import sys
import subprocess
import shutil
import serial
import signal
import re



class Tester:
    def __init__(self, platform, tmpdir, logDir):
        self.platform = platform
        self.jlinkSerial = 0
        self.executables = []
        self.tmpDir = sys.path[0] + '/' + tmpdir
        self.logFileDir = sys.path[0] + '/' + logDir
        self.dir = sys.path[0]
        self.device = ''
        if self.platform == 'Due':
            self.device='ATSAM3X8E'
        elif self.platform == 'Titan':
            self.device='STM32F413VG'
        else:
            sys.exit('Unsupported platform, exit.')
        self.result = True
        self.memIssue = False
        self.patternFail = re.compile("fail", re.IGNORECASE)
        self.patternMemoryIssue = re.compile("Memory leak in test case (.*)", re.IGNORECASE)
        self.patternResult = re.compile("Testsuite (.*) RESULT", re.IGNORECASE)
        
        self.readExec()
        self.genCommandFiles()
        self.runTests()
        self.delCommandFiles()

    def cleanup(self):
        self.delCommandFiles()

    def handler(*args):
        raise Exception("Timeout!")

    def genCommandFiles(self):
        if not os.path.exists(self.tmpDir):
            os.makedirs(self.tmpDir)
        #else:
            #sys.exit('temp dir already exists, please provide a differnt tmpdir.')
        for excecutable in self.executables:
            with open(self.dir + '/config/jlinkConfigTemplate', 'r') as ftemp:
                congigTemplate = ftemp.read()
            subst = {}
            subst['platform'] = self.platform
            subst['testName'] = excecutable
            with open(self.tmpDir + '/' + self.platform + excecutable +".jlink", 'w') as f:
                f.write(congigTemplate.format(**subst))

    def readExec(self):
        with open(self.dir + '/config/executables', "r") as file:
            for line in file:
                if (line == '\n' or line == '' or line[0] == '#'): #Skipp newline, empty line and comments.
                    continue
                else:
                    self.executables.append(line.split()[0])

    def delCommandFiles(self):
        if os.path.exists(self.tmpDir):
            shutil.rmtree(self.tmpDir)

    def runTests(self):
        signal.signal(signal.SIGALRM, self.handler)
        try: 
            if not os.path.exists(self.logFileDir):
                os.makedirs(self.logFileDir)
            with serial.Serial(port='/dev/TitanDebug'
                    ,baudrate=115200,parity=serial.PARITY_NONE
                    ,stopbits=serial.STOPBITS_ONE
                    ,bytesize=serial.EIGHTBITS) as ser, \
                    open(self.logFileDir + '/' + self.platform + "Flash.txt", 'w') as flashLogFile, \
                    open(self.logFileDir + '/' + self.platform + "Test.txt", 'w') as testLogFile:
                for test in self.executables:
                    try:
                        signal.alarm(20)
                        print("Starting Flash process.")
                        resultOfFlash = subprocess.run(['JLinkExe', '-device', self.device
                            ,'-if', 'SWD','-speed', '4000'
                            ,'-autoconnect', '1'
                            ,'-CommanderScript', self.tmpDir + '/' + self.platform + test+'.jlink'],
                            stdout=flashLogFile,stderr=flashLogFile)
                        foundResult = False
                        print("Flash finished, evaluating the result.")
                        while not foundResult:
                            line = ser.readline().decode()
                            print(line, end='')
                            testLogFile.write(line)
                            foundFail = self.patternFail.search(line)
                            foundMem = self.patternMemoryIssue.search(line)
                            foundEnd = self.patternResult.search(line)
                            if(foundFail):
                                self.result = False
                            if(foundMem):
                                self.memIssue = True
                            if(foundEnd):
                                foundResult = True
                        signal.alarm(0)
                    except Exception as e:
                        print('Timeout in testsuite ' + test + '. Message: ' + str(e))
                        self.result = False
                        continue
                ser.close()
        except serial.SerialException as error:
            print(str(error))
            self.cleanup()
            sys.exit()


        


instance = Tester('Titan','tmp','log')

print("------------- Test results: ------------")
if(instance.memIssue):
    print("Warning: There are memory leaks.")
if(instance.result):
    print("Test PASSED!")
    sys.exit(0)
else:
    print("Test FAILED, see log.")
    sys.exit(1)