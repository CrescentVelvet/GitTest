'''
Author       : velvet
Date         : 2020-11-10 16:10:56
LastEditTime : 2020-11-10 17:15:51
LastEditors  : velvet
Description  : 图像去模糊，参考https://blog.csdn.net/wsp_1138886114/article/details/95024180
'''
# coding: utf-8
from cv2 import cv2
import numpy as np
import math
import random

# 点扩散函数(旧)
def motion_process(image_size, motion_angle, motion_length):
    PSF = np.zeros(image_size)
    center_position_y = (image_size[0] - 1) / 2  # 图像变换中心点的纵坐标
    center_position_x = (image_size[1] - 1) / 2  # 图像变换中心点的横坐标
    slope_tan = math.tan(motion_angle * math.pi / 180)
    slope_cot = 1 / slope_tan
    if slope_tan <= 1:
        for i in range(motion_length):
            offset = round(i * slope_tan)
            PSF[int(center_position_y + offset), int(center_position_x - offset)] = 1
        return PSF / PSF.sum()  # 对点扩散函数进行归一化亮度
    else:
        for i in range(motion_length):
            offset = round(i * slope_cot)
            PSF[int(center_position_y - offset), int(center_position_x + offset)] = 1
        return PSF / PSF.sum()  # 对点扩散函数进行归一化亮度

# 点扩散函数(新)
def get_motion_psf(image_size, motion_angle, motion_length):
    PSF = np.zeros(image_size) # 点扩散函数PSF
    x_center = (image_size[0] - 1) / 2
    y_center = (image_size[1] - 1) / 2
    sin_val = math.sin(motion_angle * math.pi / 180)
    cos_val = math.cos(motion_angle * math.pi / 180)
    # 将对应角度上motion_length个点置成1
    for i in range(motion_length):
        x_offset = round(sin_val * i)
        y_offset = round(cos_val * i)
        PSF[int(x_center - x_offset), int(y_center + y_offset)] = 1
    return PSF / PSF.sum() # 归一化

# 我的运动模糊函数
def motion_blurred(input, PSF, eps):
    input_fft = np.fft.fft2(input)  # 进行二维数组的傅里叶变换
    PSF_fft = np.fft.fft2(PSF) + eps
    blurred = np.fft.ifft2(input_fft * PSF_fft)
    blurred = np.abs(np.fft.fftshift(blurred))  # 将零频点移到频谱的中间
    return blurred

# 网上的运动模糊函数
def motion_blur(gray, degree, angle):
    gray = np.array(gray)
    M = cv2.getRotationMatrix2D((round(degree / 2), round(degree / 2)), angle, 1)
    motion_blur_kernel = np.diag(np.ones(degree))
    motion_blur_kernel = cv2.warpAffine(motion_blur_kernel, M, (degree, degree))
    PSF = motion_blur_kernel / degree
    blurred = cv2.filter2D(gray, -1, PSF)
    blurred = cv2.normalize(blurred,None, 0, 255, cv2.NORM_MINMAX)
    blurred = np.array(blurred, dtype=np.uint8)
    return blurred,PSF

# 失焦模糊函数(高斯模糊)
def gauss_blurred(input, ksize):
    blurred = cv2.GaussianBlur(input, ksize, sigmaX=0, sigmaY=0)
    return blurred

# 添加高斯噪声函数
def add_gaussian_noise(image_in, noise_sigma):
    # temp_image = np.float64(np.copy(image_in))
    h = image_in.shape[0]
    w = image_in.shape[1]
    noise = np.random.randn(h, w) * noise_sigma
    noisy_image = np.zeros(image_in.shape, np.float64)
    if len(image_in.shape) == 2:
        noisy_image = image_in + noise
    else:
        noisy_image[:,:,0] = image_in[:,:,0] + noise
        noisy_image[:,:,1] = image_in[:,:,1] + noise
        noisy_image[:,:,2] = image_in[:,:,2] + noise
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

# 逆滤波器
def inverse_filter(input, PSF, eps): # 原模糊图像，点扩散函数，平均噪声功率
    input_fft = np.fft.fft2(input)
    PSF_fft = np.fft.fft2(PSF) + eps # 噪声功率，这是已知的，考虑epsilon
    result = np.fft.ifft2(input_fft / PSF_fft) # 计算F(u,v)的傅里叶反变换
    result = np.abs(np.fft.fftshift(result))
    return result

# 维纳滤波器
def wiener_filter(input, PSF, eps, K):  # 原模糊图像，点扩散函数，平均噪声功率，SNR
    input_fft = np.fft.fft2(input)
    PSF_fft = np.fft.fft2(PSF) + eps
    PSF_fft_1 = np.conj(PSF_fft) /(np.abs(PSF_fft)**2 + K)
    result = np.fft.ifft2(input_fft * PSF_fft_1)
    result = np.abs(np.fft.fftshift(result))
    return result

# 平滑度约束最小平方滤波器
def clear_filter(input, PSF, gamma = 0.05):
    # temp_image = np.float64(np.copy(input))
    h = input.shape[0]
    w = input.shape[1]
    kernel = np.array([[0, -1,  0],
                       [-1, 4, -1],
                       [0, -1,  0]])
    # kernel = np.array([[1,  0,  0,  0],
    #                    [-2, 1,  0,  0],
    #                    [1, -2,  1,  0],
    #                    [0,  1, -2,  1],
    #                    [0,  0,  1, -2],
    #                    [0,  0,  0,  1]])
    # 行数列数对不上需要删除
    # PSF = np.delete(PSF, -1, axis = 0)  # 删除一行
    # PSF = np.delete(PSF, -1, axis = 1)  # 删除一列
    # input = np.delete(input, -1, axis = 0)
    # input = np.delete(input, -1, axis = 1)
    # IF_noisy = np.delete(IF_noisy, -1, axis = 0)
    # IF_noisy = np.delete(IF_noisy, -1, axis = 1)
    # numerator = np.delete(numerator, -1, axis = 0)
    # numerator = np.delete(numerator, -1, axis = 1)
    # 更换为标准psf2otf函数后，行数列数对得上了
    # 运行速度也大大加快
    PSF_kernel = psf2otf(kernel, [h, w])
    numerator = np.conj(PSF)
    IF_noisy = np.fft.fft2(input)
    denominator = PSF ** 2 + gamma * (PSF_kernel ** 2)
    # print(PSF)
    # print(PSF_kernel)
    # print(denominator)
    # print(h) # 2976
    # print(w) # 3968
    # print(len(input)) # 2976
    # print(len(PSF)) # 2976
    # print(len(temp_image)) # 2976
    # print(len(numerator)) # 2976
    # print(len(IF_noisy)) # 2976
    # print(len(denominator)) # 2976
    clear_deblurred = np.fft.ifft2(numerator * IF_noisy / (denominator+1e-13)) # 除以零警告
    clear_deblurred = np.real(clear_deblurred)
    return clear_deblurred

def circShift(array,K):
    height,width = array.shape
    if len(array.shape)>=2 and height*width>=4:
        if type(K) ==int and abs(K)<height:
            updownA = array[:-K, :]
            mainArray = array[-K:,:]
            flip_matrix =  np.concatenate((mainArray,updownA),axis=0)
        elif type(K) ==tuple or type(K) ==list and abs(K[0])<height and abs(K[1])<width:
            updownA = array[:-K[0], :]
            mainArray = array[-K[0]:, :]
            temp = np.concatenate((mainArray, updownA), axis=0)

            leftrightA = temp[:, :-K[1]]
            tempArray = temp[:, -K[1]:]
            flip_matrix =  np.concatenate((tempArray,leftrightA),axis=1)
        else:
            print("移动的数组必须小于待移动的数组长与宽")
            flip_matrix = None
    else:
        print('传入数据错误或移动的Numpy.ndarray数组维度至少为(2，2)')
        flip_matrix = None
    return flip_matrix

# 网上的psf2otf函数
# def psf2otf(psf, outsize):
#     if np.count_nonzero(psf) > 1:
#         outheight, outwidth = outsize
#         psfheight, psfwidth = psf.shape[:2]
#         paddheight, paddwidth = outheight - psfheight, outwidth - psfwidth
#         # print('paddheight, paddwidth',paddheight, paddwidth)
#         # print('paddheight//2, paddwidth//2', paddheight//2, paddwidth//2)
#         if paddheight==0 and paddwidth==0:
#             otf = np.fft.fft2(psf)
#             otf = np.real(otf),0
#         else:
#             otf0 = cv2.copyMakeBorder(psf, paddheight // 2, paddheight // 2,
#                                      paddwidth // 2, paddwidth // 2,cv2.BORDER_CONSTANT)
#             K = (-(math.floor(otf0.shape[0])//2),-(math.floor(otf0.shape[1])//2))
#             otf = circShift(otf0,K)
#             otfComplex = np.fft.fft2(otf)
#             otf = np.real(otfComplex)
#     else:
#         print('该 ndrray 数组不需要转换')
#         otf =  None
#     return otf

# 我的psf2otf函数
def psf2otf(psf, outSize):
    psfSize = np.array(psf.shape)
    outSize = np.array(outSize)
    padSize = outSize - psfSize
    psf = np.pad(psf, ((0, padSize[0]), (0, padSize[1])), 'constant')
    for i in range(len(psfSize)):
        psf = np.roll(psf, -int(psfSize[i] / 2), i)
    otf = np.fft.fftn(psf)
    nElem = np.prod(psfSize)
    nOps = 0
    for k in range(len(psfSize)):
        nffts = nElem / psfSize[k]
        nOps = nOps + psfSize[k] * np.log2(psfSize[k]) * nffts
    if np.max(np.abs(np.imag(otf))) / np.max(np.abs(otf)) <= nOps * np.finfo(np.float32).eps:
        otf = np.real(otf)
    return otf

# 构造图像点扩散函数模型PSF
image = cv2.imread('picture.png', cv2.IMREAD_GRAYSCALE)
motion_length = 50  # 运动模糊的长度
motion_angle  = 30  # 运动模糊的角度
# 我的PSF函数适用于添加高斯模糊运动模糊和高斯噪声椒盐噪声，以及逆滤波维纳滤波
# PSF = get_motion_psf(image.shape, motion_angle, motion_length)
# 网上的PSF函数适用于平滑约束最小乘方滤波
blur, PF = motion_blur(image, degree=int(motion_length*0.3), angle=int(motion_angle))
PSF = psf2otf(PF, [image.shape[0], image.shape[1]])

# # 构造图像运动模糊
# image = cv2.imread('picture.png', cv2.IMREAD_GRAYSCALE)
# motion_blurred_picture_50 = np.abs(motion_blurred(image, PSF, 1e-3))
# cv2.imwrite('motion_blurred_picture_50.png', motion_blurred_picture_50)

# # 构造图像失焦模糊
# image = cv2.imread('picture.png', cv2.IMREAD_GRAYSCALE)
# ksize = (59, 59)  # 高斯分布卷积核半径
# gauss_blurred_picture = gauss_blurred(image, ksize)
# cv2.imwrite('gauss_blurred_picture.png', gauss_blurred_picture)

# # 添加均值为25的高斯噪声
# gasuss_sigma = 25
# blurred_picture = cv2.imread('gauss_blurred_picture_59.png', cv2.IMREAD_GRAYSCALE)
# noise_image = add_gaussian_noise(blurred_picture, noise_sigma=gasuss_sigma)
# cv2.imwrite('gasuss_noise_{}_gauss_blurred.png'.format(gasuss_sigma), noise_image)

# # 添加均值为50的高斯噪声
# gasuss_sigma = 50
# blurred_picture = cv2.imread('gauss_blurred_picture_59.png', cv2.IMREAD_GRAYSCALE)
# noise_image = add_gaussian_noise(blurred_picture, noise_sigma=gasuss_sigma)
# cv2.imwrite('gasuss_noise_{}_gauss_blurred.png'.format(gasuss_sigma), noise_image)

# # 添加信噪比为0.99的椒盐噪声
# salt_pepper = 0.99
# blurred_picture = cv2.imread('gauss_blurred_picture_59.png', cv2.IMREAD_GRAYSCALE)
# noise_image = add_salt_pepper(blurred_picture, noise_sigma=salt_pepper)
# cv2.imwrite('salt_pepper_{}_gauss_blurred.png'.format(salt_pepper), noise_image)

# # 添加信噪比为0.9的椒盐噪声
# salt_pepper = 0.9
# blurred_picture = cv2.imread('gauss_blurred_picture_59.png', cv2.IMREAD_GRAYSCALE)
# noise_image = add_salt_pepper(blurred_picture, noise_sigma=salt_pepper)
# cv2.imwrite('salt_pepper_{}_gauss_blurred.png'.format(salt_pepper), noise_image)

# # 使用逆滤波器处理无噪声的运动模糊图像
# in_image = cv2.imread('motion_blurred_picture_50.png', cv2.IMREAD_GRAYSCALE)
# out_image = inverse_filter(in_image, PSF, 1e-3)
# cv2.imwrite('inverse_motion_blurred_picture_50.png', out_image)

# # 使用逆滤波器处理有噪声的运动模糊图像
# in_image = cv2.imread('salt_pepper_0.9_motion_blurred.png', cv2.IMREAD_GRAYSCALE)
# out_image = inverse_filter(in_image, PSF, 0.5)
# cv2.imwrite('inverse_salt_pepper_0.9_motion_blurred.png', out_image)

# # 使用维纳滤波器处理无噪声的运动模糊图像
# in_image = cv2.imread('motion_blurred_picture_50.png', cv2.IMREAD_GRAYSCALE)
# out_image = wiener_filter(in_image, PSF, 1e-3, 1e-3)
# cv2.imwrite('wiener_motion_blurred_picture_50.png', out_image)

# # 使用维纳滤波器处理有噪声的运动模糊图像
# in_image = cv2.imread('gasuss_noise_50_motion_blurred.png', cv2.IMREAD_GRAYSCALE)
# out_image = wiener_filter(in_image, PSF, 0.1, 0.2)
# cv2.imwrite('wiener_gasuss_noise_50_motion_blurred.png', out_image)

# # 使用平滑约束滤波器处理无噪声的运动模糊图像
# in_image = cv2.imread('motion_blurred_picture_150.png', cv2.IMREAD_GRAYSCALE)
# out_image = clear_filter(in_image, PSF)
# cv2.imwrite('clear_motion_blurred_picture_150.png', out_image)

# 使用平滑约束滤波器处理有噪声的运动模糊图像
in_image = cv2.imread('salt_pepper_0.9_motion_blurred.png', cv2.IMREAD_GRAYSCALE)
out_image = clear_filter(in_image, PSF)
cv2.imwrite('clear_salt_pepper_0.9_motion_blurred.png', out_image)