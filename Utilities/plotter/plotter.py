from ast import Return
import os
import json
#import logging
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from filereader.filereader import fileReader
from axContainer.axzContainer import axContatiner

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
                data[IdToPlot[id]].updateData(time, value)
            else:
                return lines,
    for line in data:
        line.updateFig()
    return lines,

try:
    #read configuration
    configFile = open(os.getcwd()+'/plotter/plotterConfig.json')
    config = json.load(configFile)
    SubPlots = config["SubPlots"] # number of subplots to display.
    idInPlots = config["idInPlots"]
    filePathLog = os.getcwd()+config["PathLog"]
    filePathMap = os.getcwd()+config["PathMap"]
    #validate configuration
    if(SubPlots[0]*SubPlots[1] != len(idInPlots)):
        print("Config error.")
        exit()
    #Map ax id to variable name.
    idToName = {}
    with open(filePathMap) as map:
        for line in map:
            name, id, type = line.split(',')
            idToName[int(id)] = name
    #global varable used to store all data related to the graphs
    data = []
    fig, axs = plt.subplots(SubPlots[0],SubPlots[1], squeeze=False) # create the axs needed.
    axsList = axs.tolist()
    axsFlat = [item for sublist in axsList for item in sublist]
    IdToPlot = {}
    for idx, ax in enumerate(axsFlat):
        data.append(axContatiner(5000, ax, idToName[idInPlots[idx]])) #create a data object per subplot
        IdToPlot[idInPlots[idx]] = idx
    startTime = 0

    fh = fileReader(filePathLog, True)
    ani = animation.FuncAnimation(fig, update, interval=25, blit=False)
    plt.show()
except Exception as e:
    print(e)


