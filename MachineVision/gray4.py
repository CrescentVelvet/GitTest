'''
Author       : velvet
Date         : 2020-11-10 16:10:56
LastEditTime : 2020-11-20 18:23:00
LastEditors  : velvet
Description  : 相机标定，参考https://blog.csdn.net/betrapped/article/details/89247561
重投影误差，参考https://blog.csdn.net/u010128736/article/details/52875137
'''
from cv2 import cv2
import numpy as np
import glob
import matplotlib.pyplot as plt

# 设置寻找亚像素角点的参数，采用的停止准则是最大循环次数30和最大误差容限0.001
criteria = (cv2.TERM_CRITERIA_MAX_ITER | cv2.TERM_CRITERIA_EPS, 30, 0.001)

# 获取标定板角点的位置
objp = np.zeros((6 * 8, 3), np.float32)
# 将世界坐标系建在标定板上，所有点的Z坐标全部为0，所以只需要赋值x和y
objp[:, :2] = np.mgrid[0:8, 0:6].T.reshape(-1, 2)
obj_points = []  # 存储3D点
img_points = []  # 存储2D点
i = 0
for i in range(26):
    image_name = "c-" + str(i+1).zfill(4) # 读取图像，文件命名补齐函数zfill
    img  = cv2.imread(image_name + ".ppm")
    print(image_name)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    size = gray.shape[::-1]
    # 提取角点
    ret, corners = cv2.findChessboardCorners(gray, (8, 6), None)
    # print("ret = ", ret)
    # print(corners)
    if ret:
        obj_points.append(objp)
        # 找寻亚像素角点
        corners2 = cv2.cornerSubPix(gray, corners, (5, 5), (-1, -1), criteria)  # 在原角点的基础上寻找亚像素角点
        # print(corners2)
        if [corners2]:
            img_points.append(corners2)
        else:
            img_points.append(corners)
        # 把角点画出来
        cv2.drawChessboardCorners(img, (8, 6), corners, ret)  # 记住，OpenCV的绘制函数一般无返回值
        # cv2.imwrite('c-'+str(0).zfill(4)+'.png', img) # 文件命名补齐函数zfill
        # 保存绘制角度后的图像
        # cv2.imwrite(image_name +".png", img)
# print(len(img_points))
# cv2.destroyAllWindows()

# 标定
ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(obj_points, img_points, size, None, None)
# print("ret:", ret)
# print("mtx:\n", mtx) # 内参数矩阵
# print("dist:\n", dist)  # 畸变系数   distortion cofficients = (k_1,k_2,p_1,p_2,k_3)
# print("rvecs:\n", rvecs)  # 旋转向量  # 外参数
# print("tvecs:\n", tvecs ) # 平移向量  # 外参数
# print("-----------------------------------------------------")

# img = cv2.imread(images[2])
# h, w = img.shape[:2]
# newcameramtx, roi = cv2.getOptimalNewCameraMatrix(mtx,dist,(w,h),1,(w,h))#显示更大范围的图片（正常重映射之后会删掉一部分图像）
# print (newcameramtx)
# print("------------------使用undistort函数-------------------")
# dst = cv2.undistort(img,mtx,dist,None,newcameramtx)
# x,y,w,h = roi
# dst1 = dst[y:y+h,x:x+w]
# cv2.imwrite('calibresult3.png', dst1)
# print ("方法一:dst的大小为:", dst1.shape)

# 重投影误差
total_error = 0
error_point = img_points
for i in range(len(obj_points)):
    # 通过给定的内参数和外参数计算三维点投影到二维图像平面上的坐标
    img_points2, _ = cv2.projectPoints(obj_points[i], rvecs[i], tvecs[i], mtx, dist)
    # 范数求解函数
    error = cv2.norm(img_points[i], img_points2, cv2.NORM_L2) / len(img_points2)
    total_error += error
    # 误差点
    error_point[i] = img_points[i] - img_points2
plt.scatter(error_point[:][0], error_point[:][1], c='b')
plt.show()
print ("total error: ", total_error/len(obj_points))