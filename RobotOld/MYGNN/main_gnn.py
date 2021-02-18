'''
Author       : velvet
Date         : 2021-02-17 23:04:22
LastEditTime : 2021-02-19 00:50:16
LastEditors  : velvet
Description  : 
'''
import numpy as np
import pandas as pd
import torch
from torch_geometric.data import Data, InMemoryDataset, DataLoader
from sklearn.preprocessing import LabelEncoder

# 创建自己的数据集dataset
class MyDataset(InMemoryDataset):
    def __init__(self, root, transform=None, pre_transform=None):
        super(MyDataset, self).__init__(root, transform, pre_transform)
        self.data, self.slices = torch.load(self.processed_paths[0])
    @property
    # 数据集加载文件夹
    def raw_file_names(self):
        return []
    @property
    # 数据集保存文件夹
    def processed_file_names(self):
        return ['../20210117/data/my_gnn_data.dataset'] # 保存在data目录下
    # 从加载文件夹里读取数据
    def download(self):
        pass
    # 从保存文件夹里读取数据，并创建一个数据对象列表
    def process(self):
        data_list = []
        for i in df.index:
            # 获取当前行的元素个数
            i_number = df.iloc[i].size
            # 判断数据缺失
            # if np.isnan(df.enemy7_y[i]):
            #     print('Yes')
            # else:
            #     pass
            # 节点，奇偶项依次是x、y坐标
            # x = torch.tensor([[-1], [0], [1]], dtype=torch.float)
            # i_x = df[df.columns[::2]]可以吗？
            i_x = df.iloc[i,[j%2==0 for j in range(len(df.columns))]]
            print(i_x)
            i_y = df.iloc[i,[j%2==1 for j in range(len(df.columns))]]
            print(i_y)
            x = torch.tenser([i_x], dtype=torch.float)
            y = torch.tenser([i_y], dtype=torch.float)
            # 边，考虑球与车之间、车与车之间的联系
            # edge_index = torch.tensor([[0, 1, 1, 2],
                        #    [1, 0, 2, 1]], dtype=torch.long)
            # [0,0,0,0,0,1,1,1,1,1,2,2,2,2,2...]
            # [1,2,3,4,5,0,2,3,4,5,0,1,3,4,5...]
            i_list = list(range(i_number))
            i_source = [a for a in i_list for b in range(i_number)]
            print(i_source)
            i_target = []
            for i in range(i_number):
                i_target = i_target + i_list.remove(i_number)
            print(i_target)
            edge_index = torch.tensor([i_source], [i_target], dtype=torch.long)
            # 节点和边组成数据
            data = Data(x=x, edge_index=edge_index, y=y)
            # 数据添加到数据列表
            data_list.append(data)
        # 整理数据对象以增加读取速度，将所有实例连接到一个大数据对象中
        # 返回一个字典，便于从此对象重建单个实例
        data, slices = self.collate(data_list)
        # 将数据对象加载到属性中
        torch.save((data, slices), self.processed_paths[0])

df = pd.read_csv('data/2019-07-03_14-09_ER-Force-vs-TIGERs_Mannheim_data.txt',
    header=None, sep='\t', index_col=False,
    names=['ball_x','ball_y',
    'me1_x','me1_y','me2_x','me2_y','me3_x','me3_y','me4_x','me4_y','me5_x','me5_y',
    'me6_x','me6_y','me7_x','me7_y','me8_x','me8_y','me9_x','me9_y',
    'enemy1_x','enemy1_y','enemy2_x','enemy2_y','enemy3_x','enemy3_y','enemy4_x',
    'enemy4_y','enemy5_x','enemy5_y','enemy6_x','enemy6_y','enemy7_x','enemy7_y'])
df = df.reset_index()
df.columns=['index',
    'ball_x','ball_y',
    'me1_x','me1_y','me2_x','me2_y','me3_x','me3_y','me4_x','me4_y','me5_x','me5_y',
    'me6_x','me6_y','me7_x','me7_y','me8_x','me8_y','me9_x','me9_y',
    'enemy1_x','enemy1_y','enemy2_x','enemy2_y','enemy3_x','enemy3_y','enemy4_x',
    'enemy4_y','enemy5_x','enemy5_y','enemy6_x','enemy6_y','enemy7_x','enemy7_y']
# print(df)
print("----------预处理完成----------")
dataset = MyDataset(root='../')
train_dataset = dataset[:2]
valid_dataset = dataset[2:3]
test_dataset  = dataset[3:]
print(len(train_dataset), len(valid_dataset), len(test_dataset))
batch_size = 1
train_loader = DataLoader(train_dataset, batch_size=batch_size)
valid_loader = DataLoader(valid_dataset, batch_size=batch_size)
test_loader  = DataLoader(test_dataset,  batch_size=batch_size)
# num_items = df.
print("----------数据集构建完成----------")

print("----------模型构建完成----------")