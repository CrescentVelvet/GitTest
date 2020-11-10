import cv2
import numpy as np
import random
import matplotlib.pyplot as plt
import collections
import scipy.misc
import scipy.signal
import scipy.ndimage

# 添加高斯噪声函数
def add_gaussian_noise(image_in, noise_sigma):
    temp_image = np.float64(np.copy(image_in))
    h = temp_image.shape[0]
    w = temp_image.shape[1]
    noise = np.random.randn(h, w) * noise_sigma
    noisy_image = np.zeros(temp_image.shape, np.float64)
    if len(temp_image.shape) == 2:
        noisy_image = temp_image + noise
    else:
        noisy_image[:,:,0] = temp_image[:,:,0] + noise
        noisy_image[:,:,1] = temp_image[:,:,1] + noise
        noisy_image[:,:,2] = temp_image[:,:,2] + noise
    return noisy_image

# 添加椒盐噪声函数
def add_salt_pepper(image_in, noise_sigma):
    noisy_image = np.zeros(image_in.shape,np.uint8)
    noise_sigma = 1- noise_sigma
    for i in range(image_in.shape[0]):
        for j in range(image_in.shape[1]):
            rdn = random.random()
            if rdn < noise_sigma:
                noisy_image[i][j] = 0
            elif rdn > 1 - noise_sigma:
                noisy_image[i][j] = 255
            else:
                noisy_image[i][j] = image_in[i][j]
    return noisy_image

# 中值滤波函数
def medium_filter(image_in, x, y, step):
    sum_s = []
    for k in range(-int(step / 2), int(step / 2) + 1):
        for m in range(-int(step / 2), int(step / 2) + 1):
            sum_s.append(image_in[x + k][y + m])
    sum_s.sort()
    return sum_s[(int(step * step / 2) + 1)]

# 均值滤波函数
def mean_filter(image_in, x, y, step):
    sum_s = 0
    for k in range(-int(step / 2), int(step / 2) + 1):
        for m in range(-int(step / 2), int(step / 2) + 1):
            sum_s += image_in[x + k][y + m] / (step * step)
    return sum_s

# # 读取图像
# gray_image = cv2.imread('picture.png', cv2.IMREAD_GRAYSCALE)  # 读取灰度图像
# cv2.imwrite('grayscale_picture.png', gray_image)  # 输出灰度图像

# # 添加高斯噪声
# gasuss_sigma = 25
# noise_image = add_gaussian_noise(gray_image,noise_sigma=gasuss_sigma)
# cv2.imwrite('gasuss_noise_{}.png'.format(gasuss_sigma),noise_image)

# # 添加椒盐噪声
# salt_pepper = 0.9
# noise_image = add_salt_pepper(gray_image,noise_sigma=salt_pepper)
# cv2.imwrite('salt_pepper_{}.png'.format(salt_pepper),noise_image)

# 中值滤波去高斯噪声
gasuss_noise_image = cv2.imread('gasuss_noise_50.png', cv2.IMREAD_GRAYSCALE)  # 读取高斯噪声图像
medium_gasuss_noise_image = gasuss_noise_image
for i in range(0,gasuss_noise_image.shape[0]):
    for j in range(0,gasuss_noise_image.shape[1]):
        medium_gasuss_noise_image[i][j]=gasuss_noise_image[i][j]
        medium_gasuss_noise_image[i][j]=gasuss_noise_image[i][j]
medium_step = 5
for i in range(int(medium_step/2), gasuss_noise_image.shape[0]-int(medium_step/2)):
    for j in range(int(medium_step/2), gasuss_noise_image.shape[1]-int(medium_step/2)):
        medium_gasuss_noise_image[i][j] = medium_filter(gasuss_noise_image, i, j, medium_step)
cv2.imwrite('medium_gasuss_noise_image1.png',medium_gasuss_noise_image)

# 中值滤波去椒盐噪声
salt_pepper_image = cv2.imread('salt_pepper_0.9.png', cv2.IMREAD_GRAYSCALE)  # 读取椒盐噪声图像
medium_salt_pepper_image = salt_pepper_image
for i in range(0,salt_pepper_image.shape[0]):
    for j in range(0,salt_pepper_image.shape[1]):
        medium_salt_pepper_image[i][j]=salt_pepper_image[i][j]
        medium_salt_pepper_image[i][j]=salt_pepper_image[i][j]
medium_step = 5
for i in range(int(medium_step/2), salt_pepper_image.shape[0]-int(medium_step/2)):
    for j in range(int(medium_step/2), salt_pepper_image.shape[1]-int(medium_step/2)):
        medium_salt_pepper_image[i][j] = medium_filter(salt_pepper_image, i, j, medium_step)
cv2.imwrite('medium_salt_pepper_image1.png',medium_salt_pepper_image)

# 均值滤波去高斯噪声
gasuss_noise_image = cv2.imread('gasuss_noise_50.png', cv2.IMREAD_GRAYSCALE)  # 读取高斯噪声图像
mean_gasuss_noise_image = gasuss_noise_image
for i in range(0,gasuss_noise_image.shape[0]):
    for j in range(0,gasuss_noise_image.shape[1]):
        mean_gasuss_noise_image[i][j]=gasuss_noise_image[i][j]
        mean_gasuss_noise_image[i][j]=gasuss_noise_image[i][j]
mean_step = 5
for i in range(int(mean_step/2), gasuss_noise_image.shape[0]-int(mean_step/2)):
    for j in range(int(mean_step/2), gasuss_noise_image.shape[1]-int(mean_step/2)):
        mean_gasuss_noise_image[i][j] = mean_filter(gasuss_noise_image, i, j, mean_step)
cv2.imwrite('mean_gasuss_noise_image1.png',mean_gasuss_noise_image)

# 均值滤波去椒盐噪声
salt_pepper_image = cv2.imread('salt_pepper_0.9.png', cv2.IMREAD_GRAYSCALE)  # 读取椒盐噪声图像
mean_salt_pepper_image = salt_pepper_image
for i in range(0,salt_pepper_image.shape[0]):
    for j in range(0,salt_pepper_image.shape[1]):
        mean_salt_pepper_image[i][j]=salt_pepper_image[i][j]
        mean_salt_pepper_image[i][j]=salt_pepper_image[i][j]
mean_step = 5
for i in range(int(mean_step/2), salt_pepper_image.shape[0]-int(mean_step/2)):
    for j in range(int(mean_step/2), salt_pepper_image.shape[1]-int(mean_step/2)):
        mean_salt_pepper_image[i][j] = mean_filter(salt_pepper_image, i, j, mean_step)
cv2.imwrite('mean_salt_pepper_image1.png',mean_salt_pepper_image)