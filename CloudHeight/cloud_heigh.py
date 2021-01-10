'''
Author       : velvet
Date         : 2021-01-09 15:19:57
LastEditTime : 2021-01-10 21:07:12
LastEditors  : velvet
Description  : 双目视觉云高估计
'''

# coding: utf-8
from cv2 import cv2
import numpy as np
import math
import random

# 彩色图像圆形裁剪函数
def cut_circle(img):
    # 获取图像尺寸
    img_h, img_w = img.shape[:2]
    img_h = int(img_h)
    img_w = int(img_w)
    # 生成圆形显示模板
    circleIn = np.zeros((img_h, img_w, 1), np.uint8)
    circleIn = cv2.circle(circleIn, (img_w // 2, img_h // 2), min(img_h, img_w) // 2, (1), -1)
    # 生成空白图片
    img_circle = np.zeros((img_h, img_w, 4), np.uint8)
    #复制前3个通道
    img_circle[:, :, 0] = np.multiply(img[:, :, 0], circleIn[:, :, 0])
    img_circle[:, :, 1] = np.multiply(img[:, :, 1], circleIn[:, :, 0])
    img_circle[:, :, 2] = np.multiply(img[:, :, 2], circleIn[:, :, 0])
    #设置α通道的不透明部分
    circleIn[circleIn == 1] = 255
    img_circle[:, :, 3] = circleIn[:, :, 0]
    # 返回裁剪后图像
    return(img_circle)

# 读取彩色图像
# image_sc = cv2.imread('sc-20201119-1212.png', cv2.IMREAD_UNCHANGED)
# image_nb = cv2.imread('nb-20201119-1212.png', cv2.IMREAD_UNCHANGED)

# 读取灰度图像
# image_sc_gray = cv2.imread('sc-20201119-1212.png', cv2.IMREAD_GRAYSCALE)
image_nb_gray = cv2.imread('nb-20201119-1212.png', cv2.IMREAD_GRAYSCALE)

# # 图像阈值处理
# image_sc_thresh = cv2.threshold(image_sc_gray, 30, 255, cv2.THRESH_TOZERO)[1]

# # 图像圆形裁剪
# image_sc_circle = cut_circle(image_sc)
# image_nb_circle = cut_circle(image_nb)


# 边缘检测
# 对灰度图像添加高斯模糊
gauss_nb_gray = cv2.GaussianBlur(image_nb_gray, (3, 3), 0)
# canny边缘检测
canny_nb_gray = cv2.Canny(gauss_nb_gray, 50, 150)


# 输出图像
cv2.imwrite('nb_output.png', canny_nb_gray)
# cv2.imwrite('nb_output.png', image_nb_circle)