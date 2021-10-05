import os
import matplotlib.pyplot as plt
import matplotlib.animation as animation


def init():
    global xdata, ydata, lineCount, ax, line
    ax.set_ylim(0, 70)
    ax.set_xlim(0, 3)
    del xdata[:]
    del ydata[:]
    line.set_data(xdata, ydata)
    #for fileLine in file:
    #    pass
    #if len(fileLine) > 1:
    #    idStr, timeStr, valueStr = fileLine.split(',')
    #    startTime = int(timeStr)
    #line.set_data(xdata, ydata)
    return line,



def update(frame_number):
    # update the data
    global xdata, ydata, ax, line
    #with open(os.getcwd()+'/../QuadGS/build/top/gs_cli/LogFile.txt') as f:
    try:
        fileLines = file.readlines() # read all of the remaining lines at once...
    except:
        return line
    for fileLine in fileLines:
        if len(fileLine) > 1:
            try:
                idStr, timeStr, valueStr = fileLine.split(',')
            except ValueError as err:
                print("line malformatted: " + fileLine)
                return line
            id=int(idStr)
            time=(int(timeStr) - startTime)/1000 # divide by the tick rate of the fc.
            value=float(valueStr)
            xdata.append(time)
            ydata.append(value)

        else:
            return line,
    if len(fileLines) > 0:
        xdata = xdata[-5000:]
        ydata = ydata[-5000:]
    if  len(xdata) > 0:
        xmax = max(xdata)
        xmin = xmax - 9
        ymin = min(ydata)
        ymax = max(ydata)
        ax.set_xlim(xmin, xmax)
        ax.set_ylim(ymin, ymax)
        ax.figure.canvas.draw()

    line.set_data(xdata, ydata)
    return line,

fig, ax = plt.subplots()
line, = ax.plot([], [], lw=2)
ax.ticklabel_format(useOffset=False, style='plain')
ax.grid()
xdata, ydata = [], []
startTime = 0
try:
    file = open(os.getcwd()+'/../QuadGS/build/top/gs_cli/LogFile.txt')
    ani = animation.FuncAnimation(fig, update, interval=10, init_func=init, blit=True)
    plt.show()
    file.close()
except Exception as e:
    print(str(e))
