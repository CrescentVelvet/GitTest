'''
Author       : velvet
Date         : 2021-01-09 15:19:57
LastEditTime : 2021-01-10 21:02:50
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

# # 图像畸变校正
# # OpenCV像素坐标系的原点在图像左上角
# # 截图的图片为905*901，不是正方形
# center_x = (image_nb.shape[:2][0] - 5) // 2
# center_y = (image_nb.shape[:2][1] - 1) // 2
# center_radius = int(center_x*math.sqrt(2)) + 1
# center_radius_correct = int(center_radius*3.14159*0.5) + 1
# # 新建纯黑图像
# image_nb_correct = np.zeros([center_radius_correct*2, center_radius_correct*2, 3], np.uint8)
# for y in range(center_y*2):
#     for x in range(center_x*2):
#         # 原图像的xy坐标转换极坐标
#         radius = math.sqrt((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y))
#         # 极坐标下的畸变校正
#         radius_correct = center_radius * (math.asin(radius / center_radius))
#         # 校正图像的极坐标转换xy坐标
#         if radius == 0:
#             x_correct = int(center_radius_correct / math.sqrt(2))
#             y_correct = int(center_radius_correct / math.sqrt(2))
#             print(x_correct)
#         else:
#             x_correct = int((x - center_x) * (radius_correct / radius) + center_radius_correct / math.sqrt(2))
#             y_correct = int((y - center_y) * (radius_correct / radius) + center_radius_correct / math.sqrt(2))
#         image_nb_correct[x_correct, y_correct, 0] = image_nb[x, y, 0]
#         image_nb_correct[x_correct, y_correct, 1] = image_nb[x, y, 1]
#         image_nb_correct[x_correct, y_correct, 2] = image_nb[x, y, 2]

# 边缘检测
# 对灰度图像添加高斯模糊
gauss_nb_gray = cv2.GaussianBlur(image_nb_gray, (3, 3), 0)
canny_nb_gray = cv2.Canny(gauss_nb_gray, 50, 150)


# 输出图像
cv2.imwrite('nb_output.png', canny_nb_gray)
# cv2.imwrite('nb_output.png', image_nb_circle)