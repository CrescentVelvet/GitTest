import numpy as np
from pylab import *
from matplotlib import pyplot as plt
from mpl_toolkits.axisartist.axislines import Subplot

f = open("log_data.txt", "r")
x = []
y = []
z = []
for i, line in enumerate(f.readlines()):
    if i % 2 is 0:
        x.append(line.split()[0])
    if i % 2 is not 0:
        y.append(line.split()[0])
    if i % 4 is 0 & len(line.split()) > 1:
        z.append(line.split()[1])
x = list(map(float, x))
y = list(map(float, y))
if len(line.split()) > 1:
    for i in range(int(len(x)/2)):
        z[i] = list(z[i])
        print(len(z[i]))
        if len(z[i])>21:
            for j in range(17):
                del z[i][0]
            if(z[i][-1] == 'z'):
                logname = 7
            if(z[i][-1] == 'g'):
                logname = 4
            for j in range(logname):
                del z[i][-1]
            z[i] = "".join(z[i])
        else:
            z[i] = "unknown"
else:
    z.append("null")
for i in range(len(x)):
    x[i] = round(x[i], 2)
    y[i] = round(y[i], 2)
    plt.plot(x[i], y[i], 'ob', c=(np.random.rand(), np.random.rand(), np.random.rand()))
    #plt.text(x[i], y[i], (x[i],y[i]), ha='center', va='top', fontsize=6)
    #if len(line.split()) > 1:
        #plt.text(x[i], y[i], (z[int(i/2)]), ha='center', va='bottom', fontsize=7)
    #else:
        #plt.text(x[i], y[i], (z[0]), ha='center', va='bottom', fontsize=7)
plt.xlabel('x')
plt.ylabel('y')
plt.title('log analysis')
plt.show()
