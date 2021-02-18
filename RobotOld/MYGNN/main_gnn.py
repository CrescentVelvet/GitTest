'''
Author       : velvet
Date         : 2021-02-17 23:04:22
LastEditTime : 2021-02-18 13:23:58
LastEditors  : velvet
Description  : 
'''
import numpy as np
import pandas as pd
import torch
from torch_geometric.data import InMemoryDataset
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
        return ['./my_gnn_data.dataset'] # 保存在当前目录下
    # 从加载文件夹里读取数据
    def download(self):
        pass
    # 从保存文件夹里读取数据，并创建一个数据对象列表
    def process(self):
        data_list = []
        # 根据session_id对数据进行分组
        grouped = df.groupby('session_id')
        for session_id, group in tqdm(grouped):
            # 对item_id标准化
            sess_item_id = LabelEncoder().fit_transform(group.item_id)
            # 重置索引
            group = group.reset_index(drop=True)
            # 标准化item_id加入数据组
            group['sess_item_id'] = sess_item_id
            # 好复杂！！！！！！
            node_features = group.loc[group.session_id==session_id,['sess_item_id','item_id']].sort_values('sess_item_id').item_id.drop_duplicates().values
            # 扩充一个维度
            node_features = torch.LongTensor(node_features).unsqueeze(1)
            # 终点是item_id第一列
            target_nodes = group.sess_item_id.values[1:]
            # 起点是item_id第二列
            source_nodes = group.sess_item_id.values[:-1]
            # 边由起点指向终点
            edge_index = torch.tensor([source_nodes,
                                   target_nodes], dtype=torch.long)
            # 节点x为
            x = node_features
            # 节点y为0
            y = torch.FloatTensor([group.label.values[0]])
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
	names=['ball_x','ball_y','me1_x','me1_y','me2_x','me2_y','me3_x','me3_y',
	'me4_x','me4_y','me5_x','me5_y','me6_x','me6_y','me7_x','me7_y',
	'enemy1_x','enemy1_y','enemy2_x','enemy2_y','enemy3_x','enemy3_y','enemy4_x',
	'enemy4_y','enemy5_x','enemy5_y','enemy6_x','enemy6_y','enemy7_x','enemy7_y'])
df.columns=['ball_x','ball_y','me1_x','me1_y','me2_x','me2_y','me3_x','me3_y',
	'me4_x','me4_y','me5_x','me5_y','me6_x','me6_y','me7_x','me7_y',
	'enemy1_x','enemy1_y','enemy2_x','enemy2_y','enemy3_x','enemy3_y','enemy4_x',
	'enemy4_y','enemy5_x','enemy5_y','enemy6_x','enemy6_y','enemy7_x','enemy7_y']
print(df)
# buy_df = pd.read_csv('yoochoose-data/yoochoose-buys.dat', header=None)
# buy_df.columns=['session_id','timestamp','item_id','price','quantity']
# print(buy_df)
print("----------预处理完成----------")
print("----------数据集构建完成----------")
print("----------模型构建完成----------")