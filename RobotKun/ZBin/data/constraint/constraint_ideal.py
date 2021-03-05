#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import math

# config

alpha = 45 # 80 # degree
belta = 45 # 10 # degree

force_clip = 1

print("alpha, belta : ",alpha,belta)

# process

wheel = np.array([alpha, 90 + belta, 270 - belta, 360 - alpha])/180.0*math.pi

a = abs(math.cos(wheel[0]))
b = abs(math.cos(wheel[1]))
s = a+b
FORCE_LIMIT = np.array([b/s,a/s,a/s,b/s])*force_clip

# Polar Coord
aaa = np.linspace(-math.pi,math.pi,200)
mmm = np.linspace(0.1,1.4,100)
aaa,mmm = np.meshgrid(aaa,mmm)
aaa,mmm = aaa.reshape(-1),mmm.reshape(-1)
vel_x,vel_y = mmm*np.cos(aaa),mmm*np.sin(aaa)

# Euler Coord
# vel_x = np.linspace(-1,1,100)
# vel_y = np.linspace(-1,1,100)

# vx,vy = np.meshgrid(vel_x,vel_y)
# vel_x,vel_y = vx.reshape(-1),vy.reshape(-1)

###################### end
vel = np.column_stack((vel_x,vel_y))


combine_wheel = np.column_stack((np.sin(wheel),np.cos(wheel)))
wheel_vel = vel.dot(combine_wheel.T)


limit_wheel_force = wheel_vel
wheel_force = np.clip(wheel_vel,-FORCE_LIMIT,FORCE_LIMIT)

rotate_force = wheel_force.sum(axis=1)

limit_force = limit_wheel_force.dot(combine_wheel)
force = wheel_force.dot(combine_wheel)

limit_force_x = limit_force[:,0]
limit_force_y = limit_force[:,1]

force_x = force[:,0]
force_y = force[:,1]

force_mod = np.sqrt(force_x**2 + force_y**2)
vel_mod = np.sqrt(vel_x**2 + vel_y**2)

vel_angle = np.arctan2(vel_y, vel_x)
force_angle = np.arctan2(force_y, force_x)
limit_force_angle = np.arctan2(limit_force_y, limit_force_x)

diff = np.arctan2(np.sin(vel_angle-force_angle),np.cos(vel_angle-force_angle))
limit_diff = np.arctan2(np.sin(vel_angle-limit_force_angle),np.cos(vel_angle-limit_force_angle))


# origin velocity with force plot
colors = diff/diff.max()
_, ax = plt.subplots(subplot_kw={'projection': 'polar'})
ax.bar(wheel, force_mod.max(), width=3*np.pi/180)
ax.scatter(vel_angle, force_mod,c=colors,s=30,alpha=1,cmap='RdBu')
ax.set_title("velocity&force", va='bottom')
print("diff max : ",diff.max())
print("diff after limit : ",limit_diff.max())



# rotation force plot
colors = rotate_force/rotate_force.max()
_, ax = plt.subplots(subplot_kw={'projection': 'polar'})
ax.bar(wheel, vel_mod.max(), width=3*np.pi/180)
ax.scatter(vel_angle, vel_mod,c=colors,s=30,alpha=1,cmap='RdBu')
ax.set_title("rotation", va='bottom')
print("rotate max : ",rotate_force.max())



# force plot
# _, ax = plt.subplots(subplot_kw={'projection': 'polar'})
# ax.bar(wheel, force_mod.max(), width=3*np.pi/180)
# ax.scatter(force_angle, force_mod,c=colors,s=30,alpha=1,cmap='RdBu')#RdBu



plt.show()