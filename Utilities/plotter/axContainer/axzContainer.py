"""
 class to represent one axis in the plot.
"""

class axContatiner:
    def __init__(self, size, ax, name):
        self.ax = ax
        self.ax.ticklabel_format(useOffset=False, style='plain')
        self.ax.grid(False)
        self.ax.set_ylim(0, 70)
        self.ax.set_xlim(0, 3)
        self.ax.set_title(name)
        self.line, = self.ax.plot([], [], lw=2)
        self.xdata = []
        self.ydata = []
        self.id = 0
        self.size = size
        self.line.set_data(self.xdata, self.ydata)
        self.xmax = 0
        self.xmin = 0
        self.ymax = 0
        self.ymin = 0
        self.update = False
    
    def updateData(self, xdata, ydata):
        self.update = True
        self.ydata.append(ydata)
        self.xdata.append(xdata)
        self.xdata = self.xdata[-self.size:]
        self.ydata = self.ydata[-self.size:]
        if  len(self.xdata) > 0:
            if(self.xmax < xdata):
                self.xmax = xdata
            self.xmin = self.xmax - 9
            if(self.ymin > ydata):
                self.ymin = ydata
            if(self.ymax < ydata):
                self.ymax = ydata

    def updateFig(self):
        if(self.update == False):
            return
        self.ax.set_xlim(self.xmin, self.xmax)
        self.ax.set_ylim(self.ymin, self.ymax)
        self.ax.figure.canvas.draw()
        self.line.set_data(self.xdata, self.ydata)

