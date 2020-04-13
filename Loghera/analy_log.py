import torch
import torch.nn.functional as F
import numpy as np
from pylab import *
from matplotlib import pyplot as plt
from mpl_toolkits.axisartist.axislines import Subplot
from torch.autograd import Variable

f = open("log_data.txt", "r")
x_data = []
y_data = []
z = []
for i, line in enumerate(f.readlines()):
    if i % 2 is 0:
        x_data.append(line.split()[0])
    if i % 2 is not 0:
        y_data.append(line.split()[0])
    if i % 4 is 0:
        z.append(line.split()[1])
x_data = list(map(float, x_data))
y_data = list(map(float, y_data))
for i in range(len(x_data)):
    x_data[i] = round(x_data[i], 4)
    y_data[i] = round(y_data[i], 4)
xy_data = list(zip(x_data,y_data))

#
x_data1=zeros(len(x_data))
x_data2=zeros(len(x_data))
x_data3=zeros(len(x_data))
x_data4=zeros(len(x_data))
y_data1=zeros(len(y_data))
y_data2=zeros(len(y_data))
y_data3=zeros(len(y_data))
y_data4=zeros(len(y_data))
for i in range(len(x_data)):
	x_data1[i]=x_data[i]-0.2
	x_data2[i]=x_data[i]-0.2
	x_data3[i]=x_data[i]+0.2
	x_data4[i]=x_data[i]+0.2
	y_data1[i]=y_data[i]-0.2
	y_data2[i]=y_data[i]+0.2
	y_data3[i]=y_data[i]-0.2
	y_data4[i]=y_data[i]+0.2
	# print(i)
	# print(x_data[i])
	# print(x_data1[i])
	# print(x_data1)
xy_data1=list(zip(x_data1,y_data1))
xy_data2=list(zip(x_data2,y_data2))
xy_data3=list(zip(x_data3,y_data3))
xy_data4=list(zip(x_data4,y_data4))
xy_data1=torch.Tensor(xy_data1)
xy_data2=torch.Tensor(xy_data2)
xy_data3=torch.Tensor(xy_data3)
xy_data4=torch.Tensor(xy_data4)
#

# x0 = torch.Tensor(xy_data)
x0 = torch.cat((xy_data1,xy_data2,xy_data3,xy_data4),0).type(torch.FloatTensor)
y0 = torch.zeros(int(len(x0)/2))
y1 = torch.ones(len(x0)-int(len(x0)/2))
y = torch.cat((y0,y1),0).type(torch.LongTensor)
x = Variable(x0)
y = Variable(y)

# n_data = torch.ones(100,2)#生成一个100行2列的全1矩阵
# x0 = torch.normal(2*n_data,1)#利用100行两列的全1矩阵产生一个正态分布的矩阵均值和方差分别是(2*n_data,1)
# y0 = torch.zeros(100)#给x0标定标签确定其分类0
# x1 = torch.normal(-2*n_data,1)#利用同样的方法产生第二个数据类别
# y1 = torch.ones(100)#但是x1数据类别的label就标定为1
# x = torch.cat((x0,x1),0).type(torch.FloatTensor)#cat方法就是将两个数据样本聚合在一起(x0,x1),0这个属性就是第几个维度进行聚合
# y = torch.cat((y0,y1),0).type(torch.LongTensor)#y也是一样
# x = Variable(x)#将它们装载到Variable的容器里
# y = Variable(y)#将它们装载到Variable的容器里

# print(x)
# print(y)

class Net(torch.nn.Module):#开始搭建一个神经网络
	def __init__(self,n_feature,n_hidden,n_output):#神经网络初始化，设置输入曾参数，隐藏曾参数，输出层参数
		super(Net,self).__init__()#用super函数调用父类的通用初始化函数初始一下
		self.hidden = torch.nn.Linear(n_feature,n_hidden)#设置隐藏层的输入输出参数，比如说输入是n_feature,输出是n_hidden
		self.out    = torch.nn.Linear(n_hidden,n_output)#同样设置输出层的输入输出参数

	def forward(self,x):#前向计算过程
		x = F.relu(self.hidden(x))#样本数据经过隐藏层然后被Relu函数掰弯
		x = self.out(x)#经过输出层返回
		return x

net = Net(n_feature=2,n_hidden=10,n_output=2)#two classification has two n_features#实例化一个网络结构
print(net)
 
optimizer = torch.optim.SGD(net.parameters(),lr=0.02)#设置优化器参数,lr=0.002指的是学习率的大小
loss_func = torch.nn.CrossEntropyLoss()#损失函数设置为loss_function
 
plt.ion()
 
for t in range(100):
	out = net(x)#100次迭代输出
	loss = loss_func(out,y)#计算loss为out和y的差异
 
	optimizer.zero_grad()#清除一下上次梯度计算的数值
	loss.backward()#进行反向传播
	optimizer.step()#最优化迭代
 
	if t%2 == 0:
		plt.cla()
		prediction = torch.max(out,1)[1]#返回每一行中最大值的那个元素，且返回其索引torch.max()[1]，只返回最大值的每个索引
		pred_y = prediction.data.numpy().squeeze()
		target_y = y.data.numpy()
		plt.scatter(x.data.numpy()[:,0],x.data.numpy()[:,1],c=pred_y,s=100,lw=0,cmap='RdYlGn')
		accuracy = float((pred_y == target_y).astype(int).sum())/float(target_y.size)
		plt.text(1.5,-4,'Accuracy=%.2f'%accuracy,fontdict={'size':20,'color':'red'})
		plt.pause(0.1)
plt.ioff()
plt.show()
