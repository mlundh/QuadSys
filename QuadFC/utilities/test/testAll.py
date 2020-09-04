import os
import sys
import subprocess
import shutil

class Tester:
    def __init__(self, platform, tmpdir):
        self.platform = platform
        self.jlinkSerial = 0
        self.executables = []
        self.tmpDir = sys.path[0] + '/' + tmpdir
        self.dir = sys.path[0]
        self.device = ''
        if self.platform == 'Due':
            self.device='ATSAM3X8E'
        elif self.platform == 'Titan':
            self.device='STM32F413VG'
        else:
            sys.exit('Unsupported platform, exit.')
        self.readExec()
        self.genCommandFiles()
        self.runTests()
        self.delCommandFiles()


    def genCommandFiles(self):
        if not os.path.exists(self.tmpDir):
            os.makedirs(self.tmpDir)
        else:
            sys.exit('temp dir already exists, please provide a differnt tmpdir.')
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
        for test in self.executables:
            resultOfFlash = subprocess.run(['JLinkExe', '-device', self.device, '-if', 'SWD','-speed', '4000', '-autoconnect', '1', '-CommanderScript', self.tmpDir + '/' + self.platform + test+'.jlink'])

instance = Tester('Titan','tmp')

