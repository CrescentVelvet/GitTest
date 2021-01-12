#!/usr/bin/env python
# -*- coding:utf-8 -*-
import math
import numpy as np
from sklearn import linear_model
from sklearn.preprocessing import PolynomialFeatures
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d import axes3d

targetX = 0
targetY = 0

#读取数据
f = open("shoot_data.txt", "r")
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
        #去除粗大误差
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
#射死角补偿
# for i in range(len(ball_posY)):
#     ball_posY[i] = ball_posY[i] + 600
#角速度取绝对值
for i in range(len(car_rotVel)):
    car_rotVel[i] = abs(car_rotVel[i])
#转换距离
for i in range(len(car_posX)):
    car_dis.append(np.sqrt((targetX-car_posX[i])*(targetX-car_posX[i]) + (targetY-car_posY[i])*(targetY-car_posY[i])))
    ball_dis.append(np.sqrt((targetX-ball_posX[i])*(targetX-ball_posX[i]) + (targetY-ball_posY[i])*(targetY-ball_posY[i])))
shoot_input = np.zeros((len(car_dis),2))
# shoot_output = ball_posY
shoot_xita = np.zeros(len(ball_posX))
shoot_output = np.zeros(len(ball_posX))
for i in range(len(ball_posX)):
    #纵向偏差改切向偏差
    # shoot_output[i] = ball_posY[i] * np.sqrt(car_posX[i]*car_posX[i] + car_posY[i]*car_posY[i]) / car_posX[i]
    # shoot_xita[i] = math.atan(shoot_output[i]/car_dis[i])
    #偏差角度(绝对值)
    shoot_xita[i] = math.acos(((targetX-car_posX[i])*(ball_posX[i]-car_posX[i]) + (targetY-car_posY[i])*(ball_posY[i]-car_posY[i]))
        / math.sqrt(((targetX-car_posX[i])*(targetX-car_posX[i])+(targetY-car_posY[i])*(targetY-car_posY[i]))*((ball_posX[i]-car_posX[i])*(ball_posX[i]-car_posX[i])+(ball_posY[i]-car_posY[i])*(ball_posY[i]-car_posY[i]))))
shoot_output = shoot_xita
for i in range(len(car_dis)):
    shoot_input[i] = [car_rotVel[i], car_dis[i]]
# print("X:",shoot_input)
# print("Y:",shoot_output)
# print("xita:",shoot_xita)

#绘制二维图像
# fig = plt.figure()
# fig_size = np.zeros(len(shoot_output))
# for i in range(len(shoot_output)):
#     fig_size[i] = abs(shoot_output[i])
# plt.scatter(car_rotVel,car_dis,s=fig_size,c='g',alpha=0.6)
# plt.xlabel('rotVel')
# plt.ylabel('dis')
# plt.show()
#绘制三维图像
fig = plt.figure()
ax = fig.add_subplot(111,projection='3d')
ax.scatter(car_rotVel,car_dis,shoot_output,s=10,c='g',marker='^')
plt.xlabel('rotVel')
plt.ylabel('dis')
plt.show()

#建立回归模型
# linear_reg = linear_model.ARDRegression()
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
# linear_reg = linear_model.LinearRegression()
poly_reg = PolynomialFeatures(degree=3)
shoot_input_poly = poly_reg.fit_transform(shoot_input)
linear_reg = linear_model.LinearRegression()
linear_reg.fit(shoot_input_poly,shoot_output)
# print("coefficients:",linear_reg.coef_)
# print("intercept:",linear_reg.intercept_)

#测试数据
f = open("shoot_data_test.txt", "r")
temp10_test = []
car_rotVel_test = []
car_posX_test = []
car_posY_test = []
car_dis_test = []
ball_posX_test = []
ball_posY_test = []
ball_dis_test = []
for i, line in enumerate(f.readlines()):
    if len(line.split()) == 1:
        continue
    if line.split()[1] == '000000000':
        #去除粗大误差
        if (float(temp10_test[-7])<-3500) or (float(temp10_test[-7])>3500) or (float(temp10_test[-6])<-2500) or (float(temp10_test[-6])>2500):
            continue
        if (int(temp10_test[-5])!=0):
            continue
        car_posX_test.append(temp10_test[-7])
        car_posY_test.append(temp10_test[-6])
        car_rotVel_test.append(temp10_test[-3])
        ball_posX_test.append(temp10_test[-2])
        ball_posY_test.append(temp10_test[-1])
        del temp10_test[:]
    temp10_test.append(line.split()[1])
f.close()
car_rotVel_test = list(map(float, car_rotVel_test))
car_posX_test = list(map(float, car_posX_test))
car_posY_test = list(map(float, car_posY_test))
ball_posX_test = list(map(float, ball_posX_test))
ball_posY_test = list(map(float, ball_posY_test))
#射死角补偿
# for i in range(len(ball_posY_test)):
#     ball_posY_test[i] = ball_posY_test[i] + 600
#角速度取绝对值
for i in range(len(car_rotVel_test)):
    car_rotVel_test[i] = abs(car_rotVel_test[i])
#转换距离
for i in range(len(car_posX_test)):
    car_dis_test.append(np.sqrt((targetX-car_posX_test[i])*(targetX-car_posX_test[i]) + (targetY-car_posY_test[i])*(targetY-car_posY_test[i])))
    ball_dis_test.append(np.sqrt((targetX-ball_posX_test[i])*(targetX-ball_posX_test[i]) + (targetY-ball_posY_test[i])*(targetY-ball_posY_test[i])))
shoot_input_test = np.zeros((len(car_dis_test),2))
# shoot_output_test = ball_posY_test
shoot_xita_test = np.zeros(len(ball_posY_test))
shoot_output_test = np.zeros(len(ball_posY_test))
for i in range(len(ball_posY_test)):
    #纵向偏差改切向偏差
    # shoot_output_test[i] = ball_posY_test[i] * np.sqrt(car_posX_test[i]*car_posX_test[i] + car_posY_test[i]*car_posY_test[i]) / car_posX_test[i]
    # shoot_xita_test[i] = math.atan(shoot_output_test[i]/car_dis_test[i])
    #偏差角度(绝对值)
    shoot_xita_test[i] = math.acos(((targetX-car_posX_test[i])*(ball_posX_test[i]-car_posX_test[i]) + (targetY-car_posY_test[i])*(ball_posY_test[i]-car_posY_test[i]))
        / math.sqrt(((targetX-car_posX_test[i])*(targetX-car_posX_test[i])+(targetY-car_posY_test[i])*(targetY-car_posY_test[i]))*((ball_posX_test[i]-car_posX_test[i])*(ball_posX_test[i]-car_posX_test[i])+(ball_posY_test[i]-car_posY_test[i])*(ball_posY_test[i]-car_posY_test[i]))))
shoot_output_test = shoot_xita_test
for i in range(len(car_dis_test)):
    shoot_input_test[i] = [car_rotVel_test[i], car_dis_test[i]]
#print("X_test:",shoot_input_test)
#print("Y_test:",shoot_output_test)
x_pred = shoot_input_test
y_pred = np.zeros(len(x_pred))
y_pred_xita = np.zeros(len(x_pred))
ero_pred = np.zeros(len(x_pred))
for i in range(len(x_pred)):
    # y_pred[i] = linear_reg.predict([x_pred[i]])
    y_pred[i] = linear_reg.predict(poly_reg.fit_transform([x_pred[i]]))
    y_pred_xita[i] = math.atan(y_pred[i]/car_dis_test[i])
    ero_pred[i] = y_pred_xita[i] - shoot_xita_test[i]
# print("y_test:",shoot_output_test)
# print("y_pred:",y_pred)
# print("ero_pred:",ero_pred)
xita_mean = 0
for i in range(len(shoot_xita)):
    xita_mean+=abs(shoot_xita[i])
print('xita_mean: %.3f' % (xita_mean/len(shoot_xita)))
ero_sum = 0
for i in range(len(ero_pred)):
    ero_sum+=abs(ero_pred[i])
print('ero_pred: %.3f' % (ero_sum/len(ero_pred)))