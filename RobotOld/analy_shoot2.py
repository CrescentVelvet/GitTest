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
    # 角速度取绝对值
    # for i in range(len(car_rotVel)):
    #     car_rotVel[i] = abs(car_rotVel[i])
    # 坐标转换距离
    for i in range(len(car_posX)):
        car_dis.append(np.sqrt((targetX-car_posX[i])*(targetX-car_posX[i]) + (targetY-car_posY[i])*(targetY-car_posY[i])))
        ball_dis.append(np.sqrt((targetX-ball_posX[i])*(targetX-ball_posX[i]) + (targetY-ball_posY[i])*(targetY-ball_posY[i])))
    # 求偏差角度(绝对值)(弧度制)
    shoot_xita = np.zeros(len(ball_posX))
    for i in range(len(ball_posX)):
        shoot_xita[i] = math.acos(((targetX-car_posX[i])*(ball_posX[i]-car_posX[i]) + (targetY-car_posY[i])*(ball_posY[i]-car_posY[i]))
            / math.sqrt(((targetX-car_posX[i])*(targetX-car_posX[i])+(targetY-car_posY[i])*(targetY-car_posY[i]))*((ball_posX[i]-car_posX[i])*(ball_posX[i]-car_posX[i])+(ball_posY[i]-car_posY[i])*(ball_posY[i]-car_posY[i]))))
    # 弧度转换正切值tan
    shoot_tan = np.zeros(len(shoot_xita))
    for i in range(len(shoot_xita)):
        shoot_tan[i] = math.tan(shoot_xita[i])
    # 距离转换出手速度
    shoot_vel = np.zeros(len(car_dis))
    for i in range(len(car_dis)):
        if IS_SIMULATION == 1:
            shoot_vel[i] = car_dis[i] * 1.7 + 100
        else:
            shoot_vel[i] = car_dis[i] * 0.8 + 100
    # 转向角速度转换线速度
    # for i in range(len(car_rotVel)):
        # car_rotVel[i] = car_rotVel[i] * 50
    # 设置输入输出值
    # 输入为rotvel与vel的二维向量
    # shoot_input = np.zeros((len(shoot_vel),2))
    # for i in range(len(shoot_vel)):
    #     shoot_input[i] = [car_rotVel[i], shoot_vel[i]]
    # 输入为rotvel与vel的tan的一维向量
    shoot_input = np.zeros(len(shoot_vel))
    for i in range(len(shoot_vel)):
        shoot_input[i] = car_rotVel[i] / shoot_vel[i]
    # 输出
    shoot_output = np.zeros(len(ball_posX))
    shoot_output = shoot_tan
    # 输入输出tan正切转换弧度转换角度
    for i in range(len(shoot_input)):
        # shoot_input[i] = math.atan(shoot_input[i])
        # shoot_output[i] = math.atan(shoot_output[i])
        shoot_input[i] = math.degrees(math.atan(shoot_input[i]))
        shoot_output[i] = math.degrees(math.atan(shoot_output[i]))
    out = [shoot_input,shoot_output]
    return out

def drawData(filename):
    # 参数调整
    a_xielv = 22
    b_jiejv = 0.2
    # 静态绘图
    shoot_input = readData(filename)[0]
    shoot_output = readData(filename)[1]
    # 绘制二维图像
    # ax = fig.add_subplot(221)
    ax = fig.add_subplot(111)
    ax.scatter(shoot_input,shoot_output,s=5,c='g',marker='^')
    # ax.scatter(shoot_input,linear_reg.predict(shoot_input.reshape(-1,1)),s=5,c='m',marker='.')
    ax.scatter(shoot_input,a_xielv*shoot_input+b_jiejv,s=5,c='r',marker='.')
    plt.xlabel('Input')
    plt.ylabel('Output')
    # 绘制三维图像
    # ax = fig.add_subplot(222,projection='3d')
    # ax.scatter(car_rotVel,shoot_vel,shoot_output,s=10,c='g',marker='^')
    # ax.scatter(car_rotVel,shoot_vel,linear_reg.predict(shoot_input.reshape(-1,1)),s=10,c='m')
    # ax.scatter(car_rotVel,shoot_vel,a_xielv*shoot_input+b_jiejv,s=10,c='r')
    # plt.xlabel('rotVel')
    # plt.ylabel('Vel')
    # 估计参数绘图
    # ax = fig.add_subplot(223)
    # x = np.arange(0.01,0.15,0.01)
    # y1 = 22*x+0.2
    # y2 = 75.84*x-3.34
    # ax.scatter(x,y1,s=5,c='royalblue',marker='*')
    # plt.plot(x,y2,c='chartreuse')
    # ax.scatter(x[0],y1[0]-3,s=100,c='violet',marker='*')
    # ax.scatter(x[-1],y1[-1]+4,s=100,c='violet',marker='*')

fig = plt.figure()
drawData('shoot_data.txt')
plt.show()

# # 建立回归模型
# # poly_reg = PolynomialFeatures(degree=4)# 线性转换非线性回归
# # shoot_input_poly = poly_reg.fit_transform(shoot_input.reshape(-1,1))
# # linear_reg = linear_model.ARDRegression()# 各种模型
# # linear_reg = linear_model.BayesianRidge()
# # linear_reg = linear_model.ElasticNet()
# # linear_reg = linear_model.ElasticNetCV()
# # linear_reg = linear_model.Lars()
# # linear_reg = linear_model.LarsCV()
# # linear_reg = linear_model.Lasso()
# # linear_reg = linear_model.LassoCV()
# # linear_reg = linear_model.LassoLars()
# # linear_reg = linear_model.LassoLarsCV()
# # linear_reg = linear_model.LassoLarsIC()
# linear_reg = linear_model.LinearRegression()
# linear_reg.fit(shoot_input.reshape(-1,1),shoot_output)
# # linear_reg.fit(shoot_input_poly,shoot_output)
# a_xielv = linear_reg.coef_
# b_jiejv = linear_reg.intercept_
# print("coefficients:",linear_reg.coef_[0])
# print("intercept:",linear_reg.intercept_)





# # 预测结果
# x_pred = shoot_input_test
# y_pred = np.zeros(len(x_pred))
# ero_pred = np.zeros(len(x_pred))
# # y_pred = linear_reg.predict(x_pred.reshape(-1,1))
# for i in range(len(x_pred)):
#     y_pred[i] = a_xielv*x_pred[i]+b_jiejv
# # y_pred = linear_reg.predict(poly_reg.fit_transform([x_pred]))
# for i in range(len(y_pred)):
#     ero_pred[i] = y_pred[i] - shoot_output_test[i]
# # 拟合数据均值
# data_mean = 0
# for i in range(len(shoot_tan)):
#     data_mean+=abs(shoot_tan[i])
# print('data_mean: %.3f' % (data_mean/len(shoot_tan)))
# # 测试数据均值
# test_mean = 0
# for i in range(len(shoot_tan_test)):
#     test_mean+=abs(shoot_tan_test[i])
# print('test_mean: %.3f' % (test_mean/len(shoot_tan_test)))
# # 拟合误差均值
# ero_sum = 0
# for i in range(len(ero_pred)):
#     ero_sum+=abs(ero_pred[i])
# print('ero_pred: %.3f' % (ero_sum/len(ero_pred)))






