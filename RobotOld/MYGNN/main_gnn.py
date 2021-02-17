'''
Author       : velvet
Date         : 2021-02-17 23:04:22
LastEditTime : 2021-02-17 23:23:11
LastEditors  : velvet
Description  : 
'''
import torch
from torch_geometric.data import InMemoryDataset

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
    # 从保存文件夹里读取数据
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
