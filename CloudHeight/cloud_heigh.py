'''
Author       : velvet
Date         : 2021-01-09 15:19:57
LastEditTime : 2021-03-04 11:43:18
LastEditors  : velvet
Description  : 双目视觉云高估计
'''

# coding: utf-8
from cv2 import cv2
import numpy as np
import math
import random

# 图像读取函数
def read_image(name, type):
    # 读取彩色图像
    if type == 1:
        img = cv2.imread(name, cv2.IMREAD_UNCHANGED)
    # 读取灰度图像
    elif type == 0:
        img = cv2.imread(name, cv2.IMREAD_GRAYSCALE)
    else:
        print('参数错误')
    return img

# 图像裁剪函数（圆，R=380）
def cut_circle(img):
    # 设定裁剪半径
    radius = 380
    # 获取图像尺寸
    h, w = img.shape[:2]
    # 四舍五入取整
    h = round(h)
    w = round(w)
    # 生成圆形显示模板
    circleIn = np.zeros((h, w, 1), np.uint8)
    circleIn = cv2.circle(circleIn, (h//2, w//2), radius, (1), -1)
    # 生成空白图片
    img_circle = np.zeros((h, w, 4), np.uint8)
    # 若为彩色图像
    if (img[1,1]).ndim == 1:
        # 复制前三个通道
        for y in range(w):
            for x in range(h):
                # 原图像矩阵与圆形模板矩阵相乘，得到圆形图像
                img_circle[x, y, 0] = np.multiply(img[x, y, 0], circleIn[x, y, 0]) # 蓝色通道
                img_circle[x, y, 1] = np.multiply(img[x, y, 1], circleIn[x, y, 0]) # 绿色通道
                img_circle[x, y, 2] = np.multiply(img[x, y, 2], circleIn[x, y, 0]) # 红色通道
                # 设置α通道全不透明，图像底色为纯黑
                # img_circle[x,y,3] = 255
        # 设置α通道圆形部分不透明，图像底色为透明
        circleIn[circleIn == 1] = 255
        img_circle[:, :, 3] = circleIn[:, :, 0]
    # 若为黑白图像
    elif (img[1,1]).ndim == 0:
        # 复制一个通道
        for y in range(w):
            for x in range(h):
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
def image_correct(img):
    # 原图像半径
    r = 380
    # 原图像中心
    u0 = r
    v0 = r
    # 校正图像大小比例
    cor = 3
    # 校正图像长宽
    w = r*cor
    h = r*cor
    # 校正图像中心
    x0 = w/2
    y0 = h/2
    # 等距投影焦距
    f = 2*r/math.pi
    # 校正图像初始化
    image_cor = np.zeros((w, h, 4), np.uint8)
    # 若为彩色图像
    if (img[1,1]).ndim == 1:
        # xy为校正图像坐标，uv为原图像坐标
        for x in range(w):
            for y in range(h):
                # 计算校正图像上一点到校正中心的距离
                h0 = np.sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0))
                h1 = f*math.atan2(h0,f)
                if h0 == 0:
                    u = u0
                    v = v0
                else:
                    u = h1*(x-x0)/h0 + u0
                    v = h1*(y-y0)/h0 + v0
                u = round(u)
                v = round(v)
                if u>2*u0 or v>2*v0 or u<1 or v<1:
                    continue
                image_cor[x, y, 0] = img[u, v, 0] # 蓝色通道
                image_cor[x, y, 1] = img[u, v, 1] # 绿色通道
                image_cor[x, y, 2] = img[u, v, 2] # 红色通道
                image_cor[x, y, 3] = 255        # 透明度通道
    # 若为黑白图像
    elif (img[1,1]).ndim == 0:
        for x in range(w):
            for y in range(h):
                # 计算校正图像上一点到校正中心的距离
                h0 = np.sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0))
                h1 = f*math.atan2(h0,f)
                if h0 == 0:
                    u = u0
                    v = v0
                else:
                    u = h1*(x-x0)/h0 + u0
                    v = h1*(y-y0)/h0 + v0
                u = round(u)
                v = round(v)
                if u>2*u0 or v>2*v0 or u<1 or v<1:
                    continue
                image_cor[x, y] = img[u, v] # 灰度通道
    else:
        print("图像类型错误")
    return image_cor

image_sc = read_image('sc.png', 1)
image_nb = read_image('nb.png', 1)
sc_cor = image_correct(image_sc)
nb_cor = image_correct(image_nb)
image_merge = image_mosaic(sc_cor, nb_cor)

# # 图像阈值处理
# image_sc_thresh = cv2.threshold(image_sc_gray, 30, 255, cv2.THRESH_TOZERO)[1]

# 图像圆形裁剪
# image_sc_circle = cut_circle(image_sc)
# image_nb_circle = cut_circle(image_nb)
# image_merge = image_mosaic(image_sc_circle, image_nb_circle) # 水平拼接

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
cv2.imwrite('output.png', image_merge)