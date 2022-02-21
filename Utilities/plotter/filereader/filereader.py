"""
This is a wrapper class that in addition to handling a file also saves the read position betwen enter and exit. 
"""
import os
class fileReader:
    def __init__(self, fileName, ignoreHistory):
        self.thefilename = fileName
        self.lineInFile = 0
        if(ignoreHistory):
            with open(self.thefilename, 'rb') as theFile:
                try:  # catch OSError in case of a one line file 
                    theFile.seek(-2, os.SEEK_END)
                    while theFile.read(1) != b'\n':
                        theFile.seek(-2, os.SEEK_CUR)
                except OSError:
                    theFile.seek(0)
                self.lineInFile = theFile.tell()

    def __enter__(self):
        self.theFile = open(self.thefilename, 'r')
        self.theFile.seek(self.lineInFile)
        return self

    def __exit__(self, *args):
        self.lineInFile = self.theFile.tell()
        self.theFile.close()
    
    def readlines(self):
        lines = self.theFile.readlines()
        return lines