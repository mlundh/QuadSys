import os
import json
#import logging
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from filereader.filereader import fileReader
import numpy as np

def update(frame_number):
    # update the data
    lines = []
    with fh: 
        fileLines = fh.readlines() # read all of the remaining lines at once...
        for fileLine in fileLines:
            if len(fileLine) > 1:
                try:
                    idStr, timeStr, valueStr = fileLine.split(',')
                except ValueError as err:
                    print("line malformatted: " + fileLine)
                    return lines
                id=int(idStr)
                time=(int(timeStr) - startTime)/1000 # divide by the tick rate of the fc.
                value=float(valueStr)
                data.append(time, value)
            else:
                return lines,
    return lines,

try:
    configFile = open(os.getcwd()+'/plotter/plotterConfig.json')
    print("reading config:" + configFile.name)
    config = json.load(configFile)
    SubPlots = config["SubPlots"] # number of subplots to display.
    data = []
    fig, axs = plt.subplots() # create the axs needed.
    ax = axs
    ax.ticklabel_format(useOffset=False, style='plain')
    ax.grid(False)
    ax.set_ylim(0, 700)
    ax.set_xlim(0, 3000)
    line, = ax.plot([], [], lw=2)
    xdata = []
    ydata = []
    id = 0
    line.set_data(xdata, ydata)
    startTime = 0
    filePath = os.getcwd()+'/../QuadGS/build/top/gs_cli/LogFile.txt'
    fh = fileReader(filePath)
    print("reading data:" + filePath)
    ani = animation.FuncAnimation(fig, update, interval=25, blit=False)
    plt.show()
except Exception as e:
    print(e)


