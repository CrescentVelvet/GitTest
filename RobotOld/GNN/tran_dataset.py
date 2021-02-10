import numpy as np
import pandas as pd
import pickle
import csv
import os
import torch
from torch_geometric.data import Data

print("1")
print(np.random.seed(42))
df = pd.read_csv('yoochoose-data/yoochoose-clicks.dat', header=None)

buy_df = pd.read_csv('yoochoose-data/yoochoose-buys.dat', header=None)
buy_df.columns=['session_id','timestamp','item_id','price','quantity']
print(buy_df.head(20))