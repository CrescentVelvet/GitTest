import numpy as np
from matplotlib import pyplot as plt

f = open("log_data.txt", "r")

x = []
y = []
for i, line in enumerate(f.readlines()):
    if i % 2 is 0:
        x.append(line.split()[0])
    if i % 2 is not 0:
        y.append(line.split()[0])
for i in range(len(x)):
    plt.plot(x[i], y[i], 'o', c=(np.random.rand(), np.random.rand(), np.random.rand()))
plt.show()
