import numpy as np
from matplotlib import pyplot as plt

x1 = [1,2,3,4,5,]
y1 = [2,4,5,6,7,]
x2 = [5,4,3,2,1,]
y2 = [2,4,5,6,7,]
plt.title("demo")
plt.xlabel("x axis")
plt.ylabel("y axis")
plt.plot(x1,y1,"ob",c="b")
plt.plot(x2,y2,"ob",c="r")
plt.show()