'''
Author       : velvet
Date         : 2021-01-09 15:19:57
LastEditTime : 2021-03-02 22:35:55
LastEditors  : velvet
Description  : 双目视觉云高估计
'''

# coding: utf-8
from cv2 import cv2
import numpy as np
import math
import random

# 图像裁剪函数（圆，R=380）
def cut_circle(img):
    # 设定裁剪半径
    radius = 380
    # 获取图像尺寸
    img_h, img_w = img.shape[:2]
    img_h = int(img_h)
    img_w = int(img_w)
    # 生成圆形显示模板
    circleIn = np.zeros((img_h, img_w, 1), np.uint8)
    circleIn = cv2.circle(circleIn, (img_h // 2, img_w // 2), radius, (1), -1)
    # 生成空白图片
    img_circle = np.zeros((img_h, img_w, 4), np.uint8)
    # 若为彩色图像
    if (img[1,1]).ndim == 1:
        #复制前三个通道
        for y in range(img_w):
            for x in range(img_h):
                img_circle[x, y, 0] = np.multiply(img[x, y, 0], circleIn[x, y, 0])
                img_circle[x, y, 1] = np.multiply(img[x, y, 1], circleIn[x, y, 0])
                img_circle[x, y, 2] = np.multiply(img[x, y, 2], circleIn[x, y, 0])
        #设置α通道的不透明部分
        circleIn[circleIn == 1] = 255
        img_circle[:, :, 3] = circleIn[:, :, 0]
    # 若为黑白图像
    elif (img[1,1]).ndim == 0:
        # 复制一个通道
        for y in range(img_w):
            for x in range(img_h):
                # x为纵坐标，y为横坐标，原点在左上角
                img_circle[x, y] = np.multiply(img[x, y], circleIn[x, y])
                # 去除图像中过亮部分
                # if img_circle[x, y][0] > 200:
                #     img_circle[x, y] = [200, 200, 200, 200]
    else:
        print("图像类型错误")
    # 返回裁剪后图像
    return(img_circle)

# 图像拼接函数
def image_mosaic(img1,img2):
    image_merge = np.hstack((img1, img2))
    return image_merge

# 图像校正函数
# def image_correct(ratio):
#     r = 380
#     # 校正后图片的长宽
#     w = 3*r
#     h = 3*r
#     # 校正中心
#     x0 = w/2
#     y0 = h/2
#     f = 2*r/3.14159

# 读取彩色图像
image_sc = cv2.imread('sc.png', cv2.IMREAD_UNCHANGED)
image_nb = cv2.imread('nb.png', cv2.IMREAD_UNCHANGED)

# 鱼眼畸变校正correct
# 原图像半径，也是原图像中心
r = 380
# 大视角鱼眼镜头——————
# 校正图像半径
R = 3*r
# 鱼眼镜头视角
xita = math.pi/4
# 等距投影的距离
z = R*math.tan(xita)
for u in range(r):
    for v in range(r):
        h = math.sqrt(R*R-u*u-v*v)
        x = z*u/h
        y = z*v/h
        x = round(x)
        y = round(y)
        image_cor = np.zeros((R, R, 4), np.uint8)
        image_cor[x, y, 0] = image_sc[u, v, 0]
        image_cor[x, y, 1] = image_sc[u, v, 1]
        image_cor[x, y, 2] = image_sc[u, v, 2]
        image_cor[x, y, 3] = image_sc[u, v, 3]
        print(image_cor[x,y,0])
# 基于正交投影————————
# # 校正图像长宽
# w = 3*r
# h = 3*r
# # 校正图像中心
# x0 = w/2
# y0 = h/2
# # 等距投影焦距
# f = 2*r/math.pi
# for u in range(w):
#     for v in range(h):
#         # 求任意一点到校正中心的距离
#         h0 = np.sqrt((u-x0)*(u-x0)+(v-y0)*(v-y0))
#         h1 = f*math.atan2(h0,f)
#         if h0 == 0:
#             x = r
#             y = r
#         else:
#             x = h1*(u-x0)/h0 + r
#             y = h1*(v-y0)/h0 + r
#         x = round(x)
#         y = round(y)
#         if x>2*r or y>2*r or x<1 or y<1:
#             continue
#         image_cor = np.zeros((w, h, 4), np.uint8)
#         image_cor[u, v, 0] = image_sc[x, y, 0]
#         image_cor[u, v, 1] = image_sc[x, y, 1]
#         image_cor[u, v, 2] = image_sc[x, y, 2]
#         image_cor[u, v, 3] = image_sc[x, y, 2]
#         image_cor = np.uint8(image_cor)
# 读取灰度图像
# image_sc_gray = cv2.imread('sc.png', cv2.IMREAD_GRAYSCALE)
# image_nb_gray = cv2.imread('nb.png', cv2.IMREAD_GRAYSCALE)
# image_merge = image_mosaic(image_sc_gray, image_nb_gray)

# # 图像阈值处理
# image_sc_thresh = cv2.threshold(image_sc_gray, 30, 255, cv2.THRESH_TOZERO)[1]

# # 图像圆形裁剪
# image_sc_circle = cut_circle(image_sc_gray)
# image_nb_circle = cut_circle(image_nb_gray)
# hmerge = np.hstack((image_sc_circle, image_nb_circle)) #水平拼接

# 边缘检测
# 对灰度图像添加高斯模糊
# gauss_sc_gray = cv2.GaussianBlur(image_sc_circle, (3, 3), 0)
# gauss_nb_gray = cv2.GaussianBlur(image_nb_circle, (3, 3), 0)
# canny边缘检测，第三个参数用于检测边缘，第二个参数用于将断续边缘连接起来
# canny_sc_gray = cv2.Canny(gauss_sc_gray, 30, 10)
# canny_nb_gray = cv2.Canny(gauss_nb_gray, 50, 20)

# 读取边缘图像
# canny_sc_gray = cv2.imread('sc.png', cv2.IMREAD_GRAYSCALE)
# canny_nb_gray = cv2.imread('nb.png', cv2.IMREAD_GRAYSCALE)

# SIFT算法
# sift = cv2.xfeatures2d.SIFT_create()
# kp1, des1 = sift.detectAndCompute(canny_sc_gray,None)   #des是描述子
# kp2, des2 = sift.detectAndCompute(canny_nb_gray,None)  #des是描述子
# hmerge1 = np.hstack((canny_sc_gray, canny_nb_gray)) #水平拼接
# img1 = cv2.drawKeypoints(canny_sc_gray,kp1,canny_sc_gray,color=(255,0,255)) #画出特征点，并显示为红色圆圈
# img2 = cv2.drawKeypoints(canny_nb_gray,kp2,canny_nb_gray,color=(255,0,255)) #画出特征点，并显示为红色圆圈
# hmerge2 = np.hstack((img1, img2)) #水平拼接
# BFMatcher解决匹配
# bf = cv2.BFMatcher()
# matches = bf.knnMatch(des1,des2, k=2)
# 调整ratio
# good = []
# for m,n in matches:
    # if m.distance < 0.75*n.distance:
        # good.append([m])
# img3 = cv2.drawMatchesKnn(img1,kp1,img2,kp2,matches,None,flags=2)

# 输出图像
# cv2.imwrite('sc_output.png', hmerge1)
# cv2.imwrite('nb_output.png', hmerge2)
cv2.imwrite('output.png', image_cor)