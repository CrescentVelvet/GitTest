import numpy as np
import pandas as pd
import pickle
import csv
import os
import torch
from torch.nn import Sequential as Seq, Linear, ReLU
import torch.nn.functional as F
from torch_geometric.data import Data
from torch_geometric.nn import MessagePassing, TopKPooling, GraphConv, GatedGraphConv
from torch_geometric.nn import global_mean_pool as gap, global_max_pool as gmp 
from torch_geometric.utils import remove_self_loops, add_self_loops
from sklearn.preprocessing import LabelEncoder

# pandas中的各种函数
# map运算函数(函数，列表)：   对列表中的每一个元素进行函数运算
# lambda匿名函数 x：         表示一个任意变量x
# groupby分组函数(标签)：    根据标签对数据分组，相同标签值的数据放到一起
# loc选取函数[行，列]：      按照行和列的标签选取数据，取出某一行列的数据
# drop删除函数(标签)：       删除该标签的行，删除列要加个axis=1
# unique函数()：            返回数据中不同的值
# nunique函数()：           返回数据中不同值的个数
# choice选择函数(列表):      返回这个列表的随机一项
# isin过滤函数(字符串)：     返回数据中是否出现该字符串
# shuffle洗牌函数(列表)：    将列表中的所有元素随机排序

# SAGEConv层
class SAGEConv(MessagePassing):
    def __init__(self, in_channels, out_channels):
        super(SAGEConv, self).__init__(aggr='max') #  "Max" aggregation.
        self.lin = torch.nn.Linear(in_channels, out_channels)
        self.act = torch.nn.ReLU()
        self.update_lin = torch.nn.Linear(in_channels + out_channels, in_channels, bias=False)
        self.update_act = torch.nn.ReLU()
    def forward(self, x, edge_index):
        # x has shape [N, in_channels]
        # edge_index has shape [2, E]
        edge_index, _ = remove_self_loops(edge_index)
        edge_index, _ = add_self_loops(edge_index, num_nodes=x.size(0))
        return self.propagate(edge_index, size=(x.size(0), x.size(0)), x=x)
    def message(self, x_j):
        # x_j has shape [E, in_channels]
        x_j = self.lin(x_j)
        x_j = self.act(x_j)
        return x_j
    def update(self, aggr_out, x):
        # aggr_out has shape [N, out_channels]
        new_embedding = torch.cat([aggr_out, x], dim=1)
        new_embedding = self.update_lin(new_embedding)
        new_embedding = self.update_act(new_embedding)
        return new_embedding

# 神经网络

# 预处理之后，将数据转换为Dataset数据集对象
# 将session中的每一个item都视为一个节点，一个session视为一个图
# 将数据按照session_id分组进行迭代，每个组中按照item_id进行分类
class YooChooseBinaryDataset(InMemoryDataset):
    def __init__(self, root, transform=None, pre_transform=None):
        super(YooChooseBinaryDataset, self).__init__(root, transform, pre_transform)
        self.data, self.slices = torch.load(self.processed_paths[0])
    @property
    def raw_file_names(self):
        return []
    @property
    def processed_file_names(self):
        return ['../input/yoochoose_click_binary_1M_sess.dataset']
    def download(self):
        pass
    def process(self):
        data_list = []
        # process by session_id
        grouped = df.groupby('session_id')
        for session_id, group in tqdm(grouped):
            sess_item_id = LabelEncoder().fit_transform(group.item_id)
            group = group.reset_index(drop=True)
            group['sess_item_id'] = sess_item_id
            node_features = group.loc[group.session_id==session_id,['sess_item_id','item_id']].sort_values('sess_item_id').item_id.drop_duplicates().values
            node_features = torch.LongTensor(node_features).unsqueeze(1)
            target_nodes = group.sess_item_id.values[1:]
            source_nodes = group.sess_item_id.values[:-1]
            edge_index = torch.tensor([source_nodes,
                                   target_nodes], dtype=torch.long)
            x = node_features
            y = torch.FloatTensor([group.label.values[0]])
            data = Data(x=x, edge_index=edge_index, y=y)
            data_list.append(data)
        data, slices = self.collate(data_list)
        torch.save((data, slices), self.processed_paths[0])

## 数据预处理
# 读取数据与标签，并对数据进行预处理
# 对item_id进行编码，确保从0开始
df = pd.read_csv('yoochoose-data/yoochoose-clicks.dat', header=None)
df.columns = ['session_id','timestamp','item_id','category']
buy_df = pd.read_csv('yoochoose-data/yoochoose-buys.dat', header=None)
buy_df.columns=['session_id','timestamp','item_id','price','quantity']
# print(df.head(5)) # 输出前5个数据
# print(buy_df.head(5))

# 合并df数据中id相同的数据，对每一个合并尺寸判断与2的大小
# 判断结果放进valid_session，删除该列
df['valid_session'] = df.session_id.map(df.groupby('session_id')['item_id'].size() > 2)
df = df.loc[df.valid_session].drop('valid_session',axis=1)

# 二次采样减小数据量
# 从df中随机选取1000000个session_id数据
# 选取df数据中含有这些session_id数据的行
sampled_session_id = np.random.choice(df.session_id.unique(), 1000000, replace=False)
df = df.loc[df.session_id.isin(sampled_session_id)]
# print(df.nunique())
# print(buy_df.nunique())
# print(df.isna().sum())
print(df.groupby('session_id')['item_id'].size().mean())

# 
item_encoder = LabelEncoder()
df['item_id'] = item_encoder.fit_transform(df.item_id)
print(df.head())

# 检查click数据中的session_id是否出现在buy数据中
df['label'] = df.session_id.isin(buy_df.session_id)
print(df.head())
print(df.drop_duplicates('session_id')['label'].mean())

## 数据集构建
dataset = YooChooseBinaryDataset(root='../')
# 洗牌dataset
dataset = dataset.shuffle()
# 划分训练集，验证集，测试集
train_dataset = dataset[:800000]
val_dataset = dataset[800000:900000]
test_dataset = dataset[900000:]
print(len(train_dataset))
print(len(val_dataset))
print(len(test_dataset))

## 模型构建