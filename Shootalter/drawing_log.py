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
    if i % 4 is 0:
        z.append(line.split()[1])
x = list(map(double, x))
y = list(map(double, y))
for i in range(int(len(x)/2)):
    z[i] = list(z[i])
    for j in range(17):
        del z[i][0]
    for j in range(4):
        del z[i][-1]
    z[i] = "".join(z[i])
for i in range(len(x)):
    x[i] = round(x[i], 2)
    y[i] = round(y[i], 2)
    plt.plot(x[i], y[i], 'ob', c=(np.random.rand(), np.random.rand(), np.random.rand()))
    plt.text(x[i], y[i], (x[i],y[i]), ha='center', va='top', fontsize=7)
    plt.text(x[i], y[i], (z[int(i/2)]), ha='center', va='bottom', fontsize=7)
plt.xlabel('x')
plt.ylabel('y')
plt.title('log analysis')
plt.show()
