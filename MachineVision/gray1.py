import cv2
import numpy as np
import matplotlib.pyplot as plt
import collections

# 计算灰度图的直方图
def draw_histogram(grayscale):
    gray_key = []
    gray_count = []
    gray_result = []
    histogram_gray = list(grayscale.ravel())  # 将多维数组转换成一维数组
    gray = dict(collections.Counter(histogram_gray))  # 统计图像中每个灰度级出现的次数
    gray = sorted(gray.items(), key=lambda item: item[0])  # 根据灰度级大小排序
    for element in gray:
        key = list(element)[0]
        count = list(element)[1]
        gray_key.append(key)
        gray_count.append(count)
    for i in range(0, 256):
        if i in gray_key:
            num = gray_key.index(i)
            gray_result.append(gray_count[num])
        else:
            gray_result.append(0)
    gray_result = np.array(gray_result)
    return gray_result

# 直方图转化
def histogram_equalization(histogram_e, lut_e, image_e):
    sum_temp = 0
    cf = []
    for i in histogram_e:
        sum_temp += i
        cf.append(sum_temp)
    for i, v in enumerate(lut_e):
        lut_e[i] = int(255.0 * (cf[i] / sum_temp) + 0.5)
    equalization_result = lut_e[image_e]
    return equalization_result

x = []
for i in range(0, 256):  # 横坐标
    x.append(i)

gray_image = cv2.imread('picture.png', cv2.IMREAD_GRAYSCALE)  # 读取灰度图像
cv2.imwrite('grayscale_picture.png', gray_image)
histogram = draw_histogram(gray_image)  # 直方图转化
plt.bar(x, histogram)  # 绘制原图直方图
plt.savefig('efore_histogram.png')

lut = np.zeros(256, dtype=gray_image.dtype)  # 创建空的查找表,返回image类型的用0填充的数组；
result = histogram_equalization(histogram, lut, gray_image)  # 均衡化处理
cv2.imwrite('his_grey.png', result)  # 保存均衡化后图片

image_equ = cv2.imread('picture.png', cv2.COLOR_BGR2GRAY)  # 读取彩色图像
histogram_equ = draw_histogram(image_equ)  # 直方图转化
plt.bar(x, histogram_equ)  # 绘制处理后图像直方图
plt.savefig('after_histogram.png')

plt.plot(x, lut)  # 绘制灰度级变换曲线图
plt.savefig('grayscale_transformation_curve.png')

cv2.waitKey(0)
cv2.destroyAllWindows()