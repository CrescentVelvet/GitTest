'''
Author       : velvet
Date         : 2021-01-09 15:19:57
LastEditTime : 2021-03-04 17:19:27
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
                if img_circle[x, y][0] > 200:
                    img_circle[x, y] = [200, 200, 200, 200]
    else:
        print("图像类型错误")
    # 返回裁剪后图像
    return(img_circle)

# 图像拼接函数
def mosaic_image(img1,img2):
    image_merge = np.hstack((img1, img2))
    return image_merge

# 图像校正函数
def correct_image(img):
    # 原图像半径
    r = 380
    # 原图像中心
    u0 = r
    v0 = r
    # 校正图像大小比例
    cor = 3
    # 校正图像长宽
    w = cor*r
    h = cor*r
    # 校正图像中心
    x0 = w/2
    y0 = h/2
    # 鱼眼镜头半视角
    xita = math.pi/2
    # 等距投影焦距
    f = r/xita
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
                # 去除图像中过亮部分
                if image_cor[x, y][0] > 200:
                    image_cor[x, y] = [200, 200, 200, 200]
    else:
        print("图像类型错误")
    return image_cor

# 读取图像
image_sc = read_image('sc.png', 0)
image_nb = read_image('nb.png', 0)
# 校正畸变
# sc_cor = correct_image(image_sc)
# nb_cor = correct_image(image_nb)

# 检测边缘
# 对灰度图像添加高斯模糊
# gauss_sc = cv2.GaussianBlur(sc_cor, (3, 3), 0)
# gauss_nb = cv2.GaussianBlur(nb_cor, (3, 3), 0)
# 自适应阈值化，参数(原图像，最大像素，自适应方法，赋值方法，领域大小，阈值常数)
thr_sc = cv2.adaptiveThreshold(image_sc, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 7, -1)
thr_nb = cv2.adaptiveThreshold(image_nb, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 7, -1)
# canny边缘检测，第二个low阈值用于连接断续边缘，第三个high阈值用于提取边缘
canny_sc = cv2.Canny(image_sc, 30, 10)
canny_nb = cv2.Canny(image_nb, 50, 20)

# SIFT算法
sift = cv2.xfeatures2d.SIFT_create()
kp1, des1 = sift.detectAndCompute(canny_sc,None) # des是描述子
kp2, des2 = sift.detectAndCompute(canny_nb,None) # des是描述子
img1 = cv2.drawKeypoints(canny_sc_gray,kp1,canny_sc_gray,color=(255,0,255)) # 画出特征点，并显示为红色圆圈
img2 = cv2.drawKeypoints(canny_nb_gray,kp2,canny_nb_gray,color=(255,0,255)) # 画出特征点，并显示为红色圆圈
# BFMatcher解决匹配
bf = cv2.BFMatcher()
matches = bf.knnMatch(des1,des2, k=2)
# 调整ratio
good = []
for m,n in matches:
    if m.distance < 0.75*n.distance:
        good.append([m])
img3 = cv2.drawMatchesKnn(img1,kp1,img2,kp2,matches,None,flags=2)

# image_merge = mosaic_image(canny_sc, canny_nb)
# 输出图像
cv2.imwrite('cor_output.png', img3)