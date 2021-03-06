#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import math

data = np.loadtxt("constraint.txt")

fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})

ax.plot(data[:,0]/180.0*math.pi, data[:,1],'ro')


alpha = 60 # 80 # degree
belta = 60 # 10 # degree

wheel = np.array([alpha, 90 + belta, 270 - belta, 360 - alpha])/180.0*math.pi
ax.bar(wheel, data[:,1].max(), width=3*np.pi/180)


ax.set_title("model", va='bottom')
plt.show()