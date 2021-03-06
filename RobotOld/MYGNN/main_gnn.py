'''
Author       : velvet
Date         : 2021-02-17 23:04:22
LastEditTime : 2021-02-22 16:36:24
LastEditors  : velvet
Description  : 
'''
import numpy as np
import pandas as pd
import networkx as nx
import time
import torch
import torch.nn.functional as F
from dgl import DGLGraph
from dgl.data import citation_graph as citegrh
from dgl.nn.pytorch import GATConv
from torch.nn import Sequential as Seq, Linear, ReLU
from torch_geometric.nn import MessagePassing, GraphConv, TopKPooling
from torch_geometric.utils import add_self_loops, remove_self_loops, degree
from torch_geometric.data import Data, InMemoryDataset, DataLoader
from sklearn.preprocessing import LabelEncoder
embed_dim = 128


# 创建GCNConv层
class GCNConv(MessagePassing):
    def __init__(self, in_channels, out_channels):
        super(GCNConv, self).__init__(aggr='add')
        self.lin = torch.nn.Linear(in_channels, out_channels)
    def forward(self, x, edge_index):
        # Step 1: 邻接矩阵添加自循环
        edge_index, _ = add_self_loops(edge_index, num_nodes=x.size(0))
        # Step 2: 节点特征矩阵进行线性变换
        x = self.lin(x)
        # Step 3-5: 标准化节点特征，聚合邻接节点信息，得到节点新的embeddings
        return self.propagate(edge_index, x=x, norm=norm)
    def message(self, x_j, norm):
        # Step 3: 标准化节点特征
        row, col = edge_index
        deg = degree(col, x.size(0), dtype=x.dtype)
        deg_inv_sqrt = deg.pow(-0.5)
        norm = deg_inv_sqrt[row] * deg_inv_sqrt[col]
        return norm.view(-1, 1) * x_j
    def update(self, aggr_out):
        # Step 5: 得到节点新的embeddings
        return aggr_out

# 创建SAGEConv层
class SAGEConv(MessagePassing):
    def __init__(self, in_channels, out_channels):
        super(SAGEConv, self).__init__(aggr='max')
        self.lin = torch.nn.Linear(in_channels, out_channels)
        self.act = torch.nn.ReLU()
        self.update_lin = torch.nn.Linear(in_channels + out_channels, in_channels, bias=False)
        self.update_act = torch.nn.ReLU()
    def forward(self, x, edge_index):
        edge_index, _ = remove_self_loops(edge_index)
        edge_index, _ = add_self_loops(edge_index, num_nodes=x.size(0))
        return self.propagate(edge_index, size=(x.size(0), x.size(0)), x=x)
    def message(self, x_j):
        x_j = self.lin(x_j)
        x_j = self.act(x_j)
        return x_j
    def update(self, aggr_out, x):
        new_embedding = torch.cat([aggr_out, x], dim=1)
        new_embedding = self.update_lin(new_embedding)
        new_embedding = self.update_act(new_embedding)
        return new_embedding

# 创建GAT层
class GATLayer(torch.nn.Module):
    def __init__(self, g, in_dim, out_dim):
        super(GATLayer, self).__init__()
        self.g = g
        # equation (1)
        self.fc = torch.nn.Linear(in_dim, out_dim, bias=False)
        # equation (2)
        self.attn_fc = torch.nn.Linear(2 * out_dim, 1, bias=False)
        self.reset_parameters()
    def reset_parameters(self):
        """Reinitialize learnable parameters."""
        gain = torch.nn.init.calculate_gain('relu')
        torch.nn.init.xavier_normal_(self.fc.weight, gain=gain)
        torch.nn.init.xavier_normal_(self.attn_fc.weight, gain=gain)
    def edge_attention(self, edges):
        # edge UDF for equation (2)
        z2 = torch.cat([edges.src['z'], edges.dst['z']], dim=1)
        a = self.attn_fc(z2)
        return {'e': torch.nn.functional.leaky_relu(a)}
    def message_func(self, edges):
        # message UDF for equation (3) & (4)
        return {'z': edges.src['z'], 'e': edges.data['e']}
    def reduce_func(self, nodes):
        # reduce UDF for equation (3) & (4)
        # equation (3)
        alpha = F.softmax(nodes.mailbox['e'], dim=1)
        # equation (4)
        h = torch.sum(alpha * nodes.mailbox['z'], dim=1)
        return {'h': h}
    def forward(self, h):
        # equation (1)
        z = self.fc(h)
        self.g.ndata['z'] = z
        # equation (2)
        self.g.apply_edges(self.edge_attention)
        # equation (3) & (4)
        self.g.update_all(self.message_func, self.reduce_func)
        return self.g.ndata.pop('h')

# 创建多点注意力层
class MultiHeadGATLayer(torch.nn.Module):
    def __init__(self, g, in_dim, out_dim, num_heads, merge='cat'):
        super(MultiHeadGATLayer, self).__init__()
        self.heads = torch.nn.ModuleList()
        for i in range(num_heads):
            self.heads.append(GATLayer(g, in_dim, out_dim))
        self.merge = merge
    def forward(self, h):
        head_outs = [attn_head(h) for attn_head in self.heads]
        if self.merge == 'cat':
            return torch.cat(head_outs, dim=1)
        else:
            return torch.mean(torch.stack(head_outs))

# 创建GAT模型
class GAT(torch.nn.Module):
    def __init__(self, g, in_dim, hidden_dim, out_dim, num_heads):
        super(GAT, self).__init__()
        # 输入维度是hidden_dim*num_heads，因为多个输出连接在一起
        self.layer1 = MultiHeadGATLayer(g, in_dim, hidden_dim, num_heads)
        self.layer2 = MultiHeadGATLayer(g, hidden_dim * num_heads, out_dim, 1)
    def forward(self, h):
        h = self.layer1(h)
        h = F.elu(h)
        h = self.layer2(h)
        return h

# 创建自己的神经网络模型
class RobotNet(torch.nn.Module):
    # 模型结构初始化(隐层、激活函数、层结构等)
    def __init__(self):
        # 调用父类的构造函数进行初始化
        super(RobotNet, self).__init__()
        # 添加卷积层与池化层
        self.conv1 = SAGEConv(embed_dim, 128)
        self.pool1 = TopKPooling(128, ratio=0.8)
        self.conv2 = SAGEConv(128, 128)
        self.pool2 = TopKPooling(128, ratio=0.8)
        self.conv3 = SAGEConv(128, 128)
        self.pool3 = TopKPooling(128, ratio=0.8)
        # 添加嵌入层
        self.item_embedding = torch.nn.Embedding(num_embeddings=df.shape[0]+1, embedding_dim=embed_dim)
        # 添加线性层
        self.lin1 = torch.nn.Linear(256, 128)
        self.lin2 = torch.nn.Linear(128, 64)
        self.lin3 = torch.nn.Linear(64, 1)
        self.bn1 = torch.nn.BatchNorm1d(128)
        self.bn2 = torch.nn.BatchNorm1d(64)
        self.act1 = torch.nn.ReLU()
        self.act2 = torch.nn.ReLU()        
    # 网络前向传递过程，训练、测试过程中使用model(g,features)直接执行
    def forward(self, data):
        x, edge_index, batch = data.x, data.edge_index, data.batch
        x = self.item_embedding(x)
        x = x.squeeze(1)
        x = F.relu(self.conv1(x, edge_index))
        x, edge_index, _, batch, _ = self.pool1(x, edge_index, None, batch)
        x1 = torch.cat([gmp(x, batch), gap(x, batch)], dim=1)
        x = F.relu(self.conv2(x, edge_index))
        x, edge_index, _, batch, _ = self.pool2(x, edge_index, None, batch)
        x2 = torch.cat([gmp(x, batch), gap(x, batch)], dim=1)
        x = F.relu(self.conv3(x, edge_index))
        x, edge_index, _, batch, _ = self.pool3(x, edge_index, None, batch)
        x3 = torch.cat([gmp(x, batch), gap(x, batch)], dim=1)
        x = x1 + x2 + x3
        x = self.lin1(x)
        x = self.act1(x)
        x = self.lin2(x)
        x = self.act2(x)
        x = F.dropout(x, p=0.5, training=self.training)
        x = torch.sigmoid(self.lin3(x)).squeeze(1)
        return x

# 创建自己的数据集
class RobotDataset(InMemoryDataset):
    def __init__(self, root, transform=None, pre_transform=None):
        super(RobotDataset, self).__init__(root, transform, pre_transform)
        self.data, self.slices = torch.load(self.processed_paths[0])
    @property
    # 返回未经处理的数据名称列表
    def raw_file_names(self):
        return []
    @property
    # 返回已经处理的数据名称列表
    def processed_file_names(self):
        return ['../20210117/data/my_gnn_data.dataset'] # 保存在data目录下
    # 下载数据到工作目录中
    def download(self):
        pass
    # 整合数据并创建数据集列表
    def process(self):
        data_list = []
        for i in df.index:
            # -99999是初始化数字，要跳过
            
            # 提取奇数偶数项
            i_x = df.iloc[i,[j%2==1 for j in range(len(df.columns))]]
            i_y = df.iloc[i,[j%2==0 for j in range(len(df.columns))]]
            # 删除nan项
            i_x = [i_x_ for i_x_ in i_x if i_x_ == i_x_]
            i_y = [i_y_ for i_y_ in i_y if i_y_ == i_y_]
            # 获取节点个数
            i_number = len(i_x)
            # 元素化为子列表
            i_x = list(map(lambda x:[x], i_x))
            i_y = list(map(lambda y:[y], i_y))
            # 节点，奇数偶数项为x、y坐标
            x = torch.tensor(i_x, dtype=torch.float)
            y = torch.tensor(i_y, dtype=torch.float)
            # [0,0,0,0,0,1,1,1,1,1,2,2,2,2,2...]
            # [1,2,3,4,5,0,2,3,4,5,0,1,3,4,5...]
            # 基本顺序数列
            i_list = list(range(i_number))
            # 起点是单个元素重复多次的数列
            i_source = [a_i_list for a_i_list in i_list for b_i_number in range(i_number-1)]
            i_target = []
            for i_number_ in range(i_number):
                # 复制列表
                i_list_ = list(i_list)
                # 移除元素
                i_list_.remove(i_number_)
                # 终点是依次移除元素的数列
                i_target = i_target + i_list_
            # 边，考虑球与车之间、车与车之间的联系
            edge_index = torch.tensor([i_source, i_target], dtype=torch.long)
            # 节点和边组成数据
            data = Data(x=x, edge_index=edge_index, y=y)
            # 数据添加到数据列表
            data_list.append(data)
        # 整理数据对象以增加读取速度，将所有实例连接到一个大数据对象中
        # 返回一个字典，便于从此对象重建单个实例
        data, slices = self.collate(data_list)
        # 将数据对象加载到属性中
        torch.save((data, slices), self.processed_paths[0])

# 训练函数
# 将从训练集构造的DataLoader迭代，然后反向传播损失函数
def train():
    model.train()
    loss_all = 0
    for data in train_loader:
        data = data.to(device)
        optimizer.zero_grad()
        output = model(data)
        label = data.y.to(device)
        loss = criterion(output, label)
        loss.backward()
        loss_all += data.num_graphs * loss.item()
        optimizer.step()
    return loss_all / len(train_dataset)

# 估计函数
# 用AUC替代准确度，对任务完成情况进行估计评价
def evaluate(loader):
    model.eval()
    predictions = []
    labels = []
    # 设置梯度gradient归零，加速计算
    with torch.no_grad():
        for data in loader:
            data = data.to(device)
            pred = model(data).detach().cpu().numpy()
            label = data.y.detach().cpu().numpy()
            predictions.append(pred)
            labels.append(label)
    predictions = np.hstack(predictions)
    labels = np.hstack(labels)
    return roc_auc_score(labels, predictions)

# 加载Cora数据集
def load_cora_data():
    data = citegrh.load_cora()
    features = torch.FloatTensor(data.features)
    labels = torch.LongTensor(data.labels)
    mask = torch.BoolTensor(data.train_mask)
    g = DGLGraph(data.graph)
    return g, features, labels, mask

# 读取数据
df = pd.read_csv('data/medium_2019-07-03_14-09_ER-Force-vs-TIGERs_Mannheim.txt', 
    encoding='ISO-8859-1',
    header=None, sep='\t', index_col=False,
    names=['ball_x','ball_y',
    'me1_x','me1_y','me2_x','me2_y','me3_x','me3_y','me4_x','me4_y','me5_x','me5_y',
    'me6_x','me6_y','me7_x','me7_y','me8_x','me8_y','me9_x','me9_y',
    'enemy1_x','enemy1_y','enemy2_x','enemy2_y','enemy3_x','enemy3_y','enemy4_x',
    'enemy4_y','enemy5_x','enemy5_y','enemy6_x','enemy6_y','enemy7_x','enemy7_y'])
# 添加序号
df = df.reset_index()
# 添加标签
df.columns=['index',
    'ball_x','ball_y',
    'me1_x','me1_y','me2_x','me2_y','me3_x','me3_y','me4_x','me4_y','me5_x','me5_y',
    'me6_x','me6_y','me7_x','me7_y','me8_x','me8_y','me9_x','me9_y',
    'enemy1_x','enemy1_y','enemy2_x','enemy2_y','enemy3_x','enemy3_y','enemy4_x',
    'enemy4_y','enemy5_x','enemy5_y','enemy6_x','enemy6_y','enemy7_x','enemy7_y']
print("----------预处理完成----------")
# 构建数据集
dataset = RobotDataset(root='../')
# 划分训练集，验证集，测试集
length_df = df.shape[0]
train_dataset = dataset[:int(length_df/3*2-1)]
valid_dataset = dataset[int(length_df/3*2):int(length_df/6*5-1)]
test_dataset  = dataset[int(length_df/6*5):]
# 设置单批次样本数
batch_size = 128
train_loader = DataLoader(train_dataset, batch_size=batch_size)
valid_loader = DataLoader(valid_dataset, batch_size=batch_size)
test_loader  = DataLoader(test_dataset,  batch_size=batch_size)
print("----------数据集构建完成----------")
# 使用GPU训练模型
# device = torch.device('cuda')
# model = RobotNet().to(device)
# # 设置学习率0.005
# learning_rate = 0.005
# # 构造optimizer对象，使用Adam作为优化器
# optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)
# # 设置损失函数为二元交叉熵Binary Cross Entropy
# criterion = torch.nn.BCELoss()
# for epoch in range(1):
#     loss = train()
#     train_acc = evaluate(train_loader)
#     val_acc = evaluate(val_loader)    
#     test_acc = evaluate(test_loader)
#     print('Epoch: {:03d}, Loss: {:.5f}, Train Auc: {:.5f}, Val Auc: {:.5f}, Test Auc: {:.5f}'.
#           format(epoch, loss, train_acc, val_acc, test_acc))
g, features, labels, mask = load_cora_data()
# create the model, 2 heads, each head has hidden size 8
net = GAT(g,
          in_dim=features.size()[1],
          hidden_dim=8,
          out_dim=7,
          num_heads=2)
optimizer = torch.optim.Adam(net.parameters(), lr=1e-3)
dur = []
for epoch in range(30):
    if epoch >= 3:
        t0 = time.time()
    logits = net(features)
    logp = F.log_softmax(logits, 1)
    loss = F.nll_loss(logp[mask], labels[mask])
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()
    if epoch >= 3:
        dur.append(time.time() - t0)
    print("Epoch {:05d} | Loss {:.4f} | Time(s) {:.4f}".format(
        epoch, loss.item(), np.mean(dur)))
print("----------模型训练完成----------")

print("----------误差计算完成----------")