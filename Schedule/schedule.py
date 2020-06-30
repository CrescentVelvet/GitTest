#coding=utf-8
import numpy as np
import matplotlib.pyplot as plt

def velvet():
	print("link start!")
	# 读取数据
	wyf_day=[]
	wyf_t1=[]
	wyf_t2=[]
	wyf_type=[]
	size=27
	data=[]
	with open("schedule.txt", 'r') as f: # 这样不用f.close()了
		line=f.readline()
		while line:
			line = f.readline()
			eachline=line.split()
			read_data=[float(element) for element in eachline[0:4]] # 0:4读取一行数据个数
			data.append(read_data)
	data.pop()
	for index in range(len(data)):
		wyf_day.append(data[index][0])
		wyf_t1.append(data[index][1])
		wyf_t2.append(data[index][2]-data[index][1])
		if data[index][3] == 1:
			wyf_type.append('lightblue')
		elif data[index][3] == 2:
			wyf_type.append('lightgreen')
		elif data[index][3] == 3:
			wyf_type.append('red')
		elif data[index][3] == 4:
			wyf_type.append('yellow')
		elif data[index][3] == 5:
			wyf_type.append('pink')
		else:
			wyf_type.append('lightsalmon')
	# 绘制图像
	# plt.bar(x,height,width,bottom)
	plt.bar(0,0,label='sleep',fc='lightblue')
	plt.bar(0,0,label='class',fc='lightgreen')
	plt.bar(0,0,label='study',fc='red')
	plt.bar(0,0,label='nga',fc='yellow')
	plt.bar(0,0,label='bili',fc='pink')
	plt.bar(0,0,label='game',fc='lightsalmon')
	for index in range(len(data)):
		plt.bar(wyf_day[index],wyf_t2[index],bottom=wyf_t1[index],fc=wyf_type[index])
	plt.xlim(1,20)
	plt.ylim(0,24)
	plt.gca().invert_yaxis() # 坐标反向
	plt.legend()
	plt.grid(True)

if __name__=="__main__":
	velvet()
	plt.show()
