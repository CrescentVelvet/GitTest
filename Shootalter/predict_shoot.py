# !/usr/bin/env python
# -*- coding:utf-8 -*-
import math
import numpy as np
from sklearn import linear_model
from sklearn.preprocessing import PolynomialFeatures
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d import axes3d
import warnings
from time import sleep
from tqdm import tqdm
warnings.filterwarnings("ignore", category=FutureWarning, module="sklearn", lineno=1978)

targetX = 0
targetY = 0
IS_SIMULATION = 1
IS_TEST = 0 # 固定input测试output，从文件名确定转速与力度

# 读取数据
def readData(filename):
    f = open(filename, "r")
    temp10 = []
    car_dir = []
    car_posX = []
    car_posY = []
    car_velMod = []
    car_velDir = []
    car_power = []
    car_rotVel = []
    ball_posX = []
    ball_posY = []
    car_dis = []
    ball_dis = []
    # 特殊测试，读取文件名中的转速与力度
    if IS_TEST == 1:
        test_rotvel = float(filename[-9])
        test_power = 0
        for i in range(4):
            test_power += float(filename[i-8]) * pow(10,(3-i))
        test_power += 1
        print(test_power)
    # 读取文件数据
    for i, line in enumerate(f.readlines()):
        if len(line.split()) == 1:
            continue
        if line.split()[1] == '000000000':
            # 去除转速偏差
            if math.fabs(float(temp10[-3])) < 0.5:
                continue
            if IS_TEST == 1:
                if math.fabs(float(temp10[-3])-test_rotvel) > 0.12:
                    continue
            car_dir.append(temp10[-9])
            car_posX.append(temp10[-8])
            car_posY.append(temp10[-7])
            car_velMod.append(temp10[-6])
            car_velDir.append(temp10[-5])
            car_power.append(temp10[-4])
            car_rotVel.append(temp10[-3])
            ball_posX.append(temp10[-2])
            ball_posY.append(temp10[-1])
            del temp10[:]
        temp10.append(line.split()[1])
    f.close()
    car_dir = list(map(float,car_dir))
    car_posX = list(map(float,car_posX))
    car_posY = list(map(float,car_posY))
    car_velMod = list(map(float,car_velMod))
    car_velDir = list(map(float,car_velDir))
    car_power = list(map(float,car_power))
    car_rotVel = list(map(float,car_rotVel))
    ball_posX = list(map(float,ball_posX))
    ball_posY = list(map(float,ball_posY))
    # 输入input为rotvel与power的角度值
    shoot_input = np.zeros(len(car_rotVel))
    for i in range(len(car_rotVel)):
        if IS_TEST == 1:
            shoot_input[i] = math.degrees(math.atan(car_rotVel[i]/test_power))
        else:
            shoot_input[i] = math.degrees(math.atan(car_rotVel[i]/car_power[i]))
        # shoot_input[i] = car_power[i]
    # 输出output
    shoot_output = np.zeros(len(car_posX))
    for i in tqdm(range(len(car_posX))):
        shoot_output[i] = math.degrees(math.atan((ball_posY[i]-car_posY[i])/(ball_posX[i]-car_posX[i]))) - math.degrees(car_dir[i])
        if shoot_output[i] > 160:
            shoot_output[i] -= 180
        if shoot_output[i] < -160:
            shoot_output[i] += 180
    # 求个输入输出平均值
    average_input = 0
    average_output = 0
    for i in range(len(shoot_output)):
        average_input += shoot_input[i]
        average_output += shoot_output[i]
    average_input /= len(shoot_input)
    average_output /= len(shoot_output)
    # 数据读取
    out = [shoot_input,shoot_output,average_input,average_output]
    return out

def drawData(filename,num):
    a_xielv = 138
    b_jiejv = 0
    ax = fig.add_subplot(num)
    out = readData(filename)
    shoot_input = out[0]
    shoot_output = out[1]
    average_input = out[2]
    average_output = out[3]
    if IS_TEST == 1:
        a_xielv = 0
        b_jiejv = average_output
    # 建立回归模型
    poly_reg = PolynomialFeatures(degree=1)# 线性转换非线性回归
    shoot_input_poly = poly_reg.fit_transform(shoot_input.reshape(-1,1))
    # linear_reg = linear_model.ARDRegression()# 各种模型
    # linear_reg = linear_model.BayesianRidge()
    # linear_reg = linear_model.ElasticNet()
    # linear_reg = linear_model.ElasticNetCV()
    # linear_reg = linear_model.Lars()
    # linear_reg = linear_model.LarsCV()
    # linear_reg = linear_model.Lasso()
    # linear_reg = linear_model.LassoCV()
    # linear_reg = linear_model.LassoLars()
    # linear_reg = linear_model.LassoLarsCV()
    # linear_reg = linear_model.LassoLarsIC()
    linear_reg = linear_model.LinearRegression()
    linear_reg.fit(shoot_input.reshape(-1,1),shoot_output)
    ax.scatter(shoot_input,shoot_output,s=5,c='limegreen',marker='^')
    ax.scatter(shoot_input,a_xielv*shoot_input+b_jiejv,s=5,c='orangered',marker='.')
    if IS_TEST != 1:
        ax.scatter(shoot_input,linear_reg.predict(shoot_input.reshape(-1,1)),s=5,c='violet',marker='.')
    ax.set_title(filename)
    # ax.set_xlabel(round(linear_reg.coef_[0],2))
    # ax.set_ylabel(round(linear_reg.intercept_,2))
    ax.set_xlabel(average_input)
    ax.set_ylabel(average_output)
    # plt.xlim(0,0.16)
    plt.ylim(-10,20)

# plt.ion()
fig = plt.figure()
drawData('shoot_data*22000.txt',231)
drawData('shoot_data*32000.txt',232)
drawData('shoot_data*32500.txt',233)
drawData('shoot_data*33000.txt',234)
drawData('shoot_data*33500.txt',235)
drawData('shoot_data.txt',236)
plt.show()