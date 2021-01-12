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

# 读取数据
def readData(filename):
    f = open(filename, "r")
    temp10 = []
    car_rotVel = []
    car_posX = []
    car_posY = []
    car_dis = []
    ball_posX = []
    ball_posY = []
    ball_dis = []
    for i, line in enumerate(f.readlines()):
        if len(line.split()) == 1:
            continue
        if line.split()[1] == '000000000':
            # 去除粗大误差
            if (float(temp10[-7])<-3500) or (float(temp10[-7])>3500) or (float(temp10[-6])<-2500) or (float(temp10[-6])>2500):
                continue
            if (int(temp10[-5])!=0):
                continue
            car_posX.append(temp10[-7])
            car_posY.append(temp10[-6])
            car_rotVel.append(temp10[-3])
            ball_posX.append(temp10[-2])
            ball_posY.append(temp10[-1])
            del temp10[:]
        temp10.append(line.split()[1])
    f.close()
    car_rotVel = list(map(float,car_rotVel))
    car_posX = list(map(float,car_posX))
    car_posY = list(map(float,car_posY))
    ball_posX = list(map(float,ball_posX))
    ball_posY = list(map(float,ball_posY))
    # 坐标转换距离
    for i in range(len(car_posX)):
        car_dis.append(np.sqrt((targetX-car_posX[i])*(targetX-car_posX[i]) + (targetY-car_posY[i])*(targetY-car_posY[i])))
        ball_dis.append(np.sqrt((targetX-ball_posX[i])*(targetX-ball_posX[i]) + (targetY-ball_posY[i])*(targetY-ball_posY[i])))
    # 求偏差弧度绝对值
    shoot_xita = np.zeros(len(ball_posX))
    for i in range(len(ball_posX)):
        shoot_xita[i] = math.acos(((targetX-car_posX[i])*(ball_posX[i]-car_posX[i]) + (targetY-car_posY[i])*(ball_posY[i]-car_posY[i]))
            / math.sqrt(((targetX-car_posX[i])*(targetX-car_posX[i])+(targetY-car_posY[i])*(targetY-car_posY[i]))*((ball_posX[i]-car_posX[i])*(ball_posX[i]-car_posX[i])+(ball_posY[i]-car_posY[i])*(ball_posY[i]-car_posY[i]))))
    # 求偏差弧度正负性
    for i in range(len(shoot_xita)):
        shoot_xita[i] = np.sign((targetX-car_posX[i])*(ball_posY[i]-car_posY[i])-(targetY-car_posY[i])*(ball_posX[i]-car_posX[i])) * shoot_xita[i]
    # 距离转换出手速度
    shoot_vel = np.zeros(len(car_dis))
    for i in range(len(car_dis)):
        if IS_SIMULATION == 1:
            shoot_vel[i] = car_dis[i] * 1.7 + 100
        else:
            shoot_vel[i] = car_dis[i] * 0.8 + 100
    # 输入为rotvel与vel的tan的一维向量
    shoot_input = np.zeros(len(shoot_vel))
    for i in range(len(shoot_vel)):
        shoot_input[i] = car_rotVel[i] / shoot_vel[i]
        shoot_input[i] = math.atan(shoot_input[i])
    # 输出
    shoot_output = np.zeros(len(ball_posX))
    shoot_output = shoot_xita
    # 输入输出弧度转换角度
    for i in range(len(shoot_output)):
        shoot_input[i] = math.degrees(shoot_input[i])
        shoot_output[i] = math.degrees(shoot_output[i])
    # 去除粗大误差
    shoot_average = 0
    for i in range(len(shoot_output)):
        shoot_average += shoot_output[i]
    shoot_average /= len(shoot_output)
    # print(shoot_average)
    for i in tqdm(range(len(shoot_output))):
        if abs(shoot_average - shoot_output[i]) > 15:
            np.delete(shoot_output,i)
            np.delete(shoot_input,i)
            # print(shoot_output[i])
        # sleep(0.0001)

    out = [shoot_input,shoot_output]
    return out

def drawData(filename,num):
    a_xielv = 22
    b_jiejv = 0.2
    ax = fig.add_subplot(num)
    shoot_input = readData(filename)[0]
    shoot_output = readData(filename)[1]
    # 建立回归模型
    poly_reg = PolynomialFeatures(degree=4)# 线性转换非线性回归
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
    ax.scatter(shoot_input,shoot_output,s=5,c='g',marker='^')
    ax.scatter(shoot_input,a_xielv*shoot_input+b_jiejv,s=5,c='r',marker='.')
    ax.scatter(shoot_input,linear_reg.predict(shoot_input.reshape(-1,1)),s=5,c='m',marker='.')
    ax.set_title(filename)
    ax.set_xlabel(round(linear_reg.coef_[0],2))
    ax.set_ylabel(round(linear_reg.intercept_,2))
    plt.ylim(-30,30)

fig = plt.figure()
drawData('shoot_data_00000.txt',231)
drawData('shoot_data_04002.txt',232)
drawData('shoot_data_27002.txt',233)
drawData('shoot_data_76020.txt',234)
drawData('shoot_data_77010.txt',235)
drawData('shoot_data.txt',236)
plt.show()