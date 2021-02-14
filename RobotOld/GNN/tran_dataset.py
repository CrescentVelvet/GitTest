import numpy as np
import pandas as pd
import pickle
import csv
import os
import torch
from torch_geometric.data import Data
from sklearn.preprocessing import LabelEncoder

# 数据读取
df = pd.read_csv('yoochoose-data/yoochoose-clicks.dat', header=None)
df.columns = ['session_id','timestamp','item_id','category']
buy_df = pd.read_csv('yoochoose-data/yoochoose-buys.dat', header=None)
buy_df.columns=['session_id','timestamp','item_id','price','quantity']
# print(df.head(5)) # 输出前5个数据
# print(buy_df.head(5))
df['valid_session'] = df.session_id.map(df.groupby('session_id')['item_id'].size() > 2)
df = df.loc[df.valid_session].drop('valid_session',axis=1)
# 二次采样
sampled_session_id = np.random.choice(df.session_id.unique(), 1000000, replace=False)
df = df.loc[df.session_id.isin(sampled_session_id)]
# print(df.nunique()) # 输出数据中各种类别的个数
# print(buy_df.nunique())
# print(df.isna().sum())
# 检查click数据集中的session_id是否出现在buy数据集中
df['label'] = df.session_id.isin(buy_df.session_id)
print(df.head())
