import argparse
import math
import time

import torch
import torch.nn as nn
from net import gtnet
import numpy as np
import importlib

from util import *
from trainer import Optim
import os 
os.environ['CUDA_VISIBLE_DEVICES'] = '0'

# 估计函数
def evaluate(data, X, Y, model, evaluateL2, evaluateL1, batch_size):
    model.eval()
    total_loss = 0
    total_loss_l1 = 0
    n_samples = 0
    predict = None
    test = None
    # 加载数据生成batch
    for X, Y in data.get_batches(X, Y, batch_size, False):
        # 扩充数据维度，从dim位置增加一个维度
        X = torch.unsqueeze(X,dim=1)
        # 高维数据的矩阵转置，交换2和3号坐标轴
        X = X.transpose(2,3)
        # 设置不计算梯度gradient，加速计算
        with torch.no_grad():
            output = model(X)
        # 压缩数据维度，从dim位置减去一个维度
        output = torch.squeeze(output)
        # 如果输出长度为1，则增加维度
        if len(output.shape)==1:
            output = output.unsqueeze(dim=0)
        # 如果预测为无，令其等于输出
        if predict is None:
            predict = output
            test = Y
        # 否则令预测等于预测和输出的拼接
        else:
            predict = torch.cat((predict, output))
            test = torch.cat((test, Y))
        scale = data.scale.expand(output.size(0), data.m)
        total_loss += evaluateL2(output * scale, Y * scale).item()
        total_loss_l1 += evaluateL1(output * scale, Y * scale).item()
        n_samples += (output.size(0) * data.m)

    rse = math.sqrt(total_loss / n_samples) / data.rse
    rae = (total_loss_l1 / n_samples) / data.rae

    predict = predict.data.cpu().numpy()
    Ytest = test.data.cpu().numpy()
    sigma_p = (predict).std(axis=0)
    sigma_g = (Ytest).std(axis=0)
    mean_p = predict.mean(axis=0)
    mean_g = Ytest.mean(axis=0)
    index = (sigma_g != 0)
    correlation = ((predict - mean_p) * (Ytest - mean_g)).mean(axis=0) / (sigma_p * sigma_g)
    correlation = (correlation[index]).mean()
    return rse, rae, correlation

# 训练函数
def train(data, X, Y, model, criterion, optim, batch_size):
    model.train()
    total_loss = 0
    n_samples = 0
    iter = 0
    # 加载数据生成batch
    for X, Y in data.get_batches(X, Y, batch_size, True):
        # 设置模型的梯度参数为0
        model.zero_grad()
        X = torch.unsqueeze(X,dim=1)
        X = X.transpose(2,3)
        if iter % args.step_size == 0:
            # 对0到range(args.num_nodes)的顺序数字进行随机排序生成数列
            perm = np.random.permutation(range(args.num_nodes))
        num_sub = int(args.num_nodes / args.num_split)
        for j in range(args.num_split):
            if j != args.num_split - 1:
                id = perm[j * num_sub:(j + 1) * num_sub]
            else:
                id = perm[j * num_sub:]
            id = torch.tensor(id).to(device)
            tx = X[:, :, id, :]
            ty = Y[:, id]
            output = model(tx,id)
            output = torch.squeeze(output)
            scale = data.scale.expand(output.size(0), data.m)
            scale = scale[:,id]
            loss = criterion(output * scale, ty * scale)
            loss.backward()
            total_loss += loss.item()
            n_samples += (output.size(0) * data.m)
            grad_norm = optim.step()
        if iter%100==0:
            # 输出损失误差
            print('iter:{:3d} | loss: {:.3f}'.format(iter,loss.item()/(output.size(0) * data.m)))
        iter += 1
    return total_loss / n_samples

# 创建解析器——ArgumentParser对象，该对象包含将命令行解析成Python数据类型所需的全部信息
parser = argparse.ArgumentParser(description='PyTorch Time series forecasting')
# 添加参数：--可选参数名称，type=参数类型，default=参数默认值，help=参数说明
# 数据地址
parser.add_argument('--data', type=str, default='./data/solar_AL.txt',
                    help='location of the data file')
# 时间间隔
parser.add_argument('--log_interval', type=int, default=2000, metavar='N',
                    help='report interval')
# 模型保存路径
parser.add_argument('--save', type=str, default='model/model.pt',
                    help='path to save the final model')
parser.add_argument('--optim', type=str, default='adam')
parser.add_argument('--L1Loss', type=bool, default=True)
parser.add_argument('--normalize', type=int, default=2)
parser.add_argument('--device',type=str,default='cuda:1',help='')
parser.add_argument('--gcn_true', type=bool, default=True, help='whether to add graph convolution layer')
parser.add_argument('--buildA_true', type=bool, default=True, help='whether to construct adaptive adjacency matrix')
parser.add_argument('--gcn_depth',type=int,default=2,help='graph convolution depth')
parser.add_argument('--num_nodes',type=int,default=137,help='number of nodes/variables')
parser.add_argument('--dropout',type=float,default=0.3,help='dropout rate')
parser.add_argument('--subgraph_size',type=int,default=20,help='k')
parser.add_argument('--node_dim',type=int,default=40,help='dim of nodes')
parser.add_argument('--dilation_exponential',type=int,default=2,help='dilation exponential')
parser.add_argument('--conv_channels',type=int,default=16,help='convolution channels')
parser.add_argument('--residual_channels',type=int,default=16,help='residual channels')
parser.add_argument('--skip_channels',type=int,default=32,help='skip channels')
parser.add_argument('--end_channels',type=int,default=64,help='end channels')
parser.add_argument('--in_dim',type=int,default=1,help='inputs dimension')
parser.add_argument('--seq_in_len',type=int,default=24*7,help='input sequence length')
parser.add_argument('--seq_out_len',type=int,default=1,help='output sequence length')
parser.add_argument('--horizon', type=int, default=3)
parser.add_argument('--layers',type=int,default=5,help='number of layers')
parser.add_argument('--batch_size',type=int,default=32,help='batch size')
parser.add_argument('--lr',type=float,default=0.0001,help='learning rate')
parser.add_argument('--weight_decay',type=float,default=0.00001,help='weight decay rate')
parser.add_argument('--clip',type=int,default=5,help='clip')
parser.add_argument('--propalpha',type=float,default=0.05,help='prop alpha')
parser.add_argument('--tanhalpha',type=float,default=3,help='tanh alpha')
parser.add_argument('--epochs',type=int,default=1,help='')
parser.add_argument('--num_split',type=int,default=1,help='number of splits for graphs')
parser.add_argument('--step_size',type=int,default=100,help='step_size')
# 解析参数，返回args实例
args = parser.parse_args()
# 设置GPU设备
device = torch.device(args.device)
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu' )
# 设置线程数量
torch.set_num_threads(3)

# 主函数
def main():
    # 读取数据
    Data = DataLoaderS(args.data, 0.6, 0.2, device, args.horizon, args.seq_in_len, args.normalize)
    # 生成神经网络模型
    model = gtnet(args.gcn_true, args.buildA_true, args.gcn_depth, args.num_nodes,
                  device, dropout=args.dropout, subgraph_size=args.subgraph_size,
                  node_dim=args.node_dim, dilation_exponential=args.dilation_exponential,
                  conv_channels=args.conv_channels, residual_channels=args.residual_channels,
                  skip_channels=args.skip_channels, end_channels= args.end_channels,
                  seq_length=args.seq_in_len, in_dim=args.in_dim, out_dim=args.seq_out_len,
                  layers=args.layers, propalpha=args.propalpha, tanhalpha=args.tanhalpha, layer_norm_affline=False)
    # 将生成模型加载到相应设备中
    model = model.to(device)
    # 输出模型args实例
    print(args)
    print('The recpetive field size is', model.receptive_field)
    nParams = sum([p.nelement() for p in model.parameters()])
    print('Number of model parameters is', nParams, flush=True)

    if args.L1Loss:
        criterion = nn.L1Loss(size_average=False).to(device)
    else:
        criterion = nn.MSELoss(size_average=False).to(device)
    evaluateL2 = nn.MSELoss(size_average=False).to(device)
    evaluateL1 = nn.L1Loss(size_average=False).to(device)

    best_val = 10000000
    optim = Optim(
        model.parameters(), args.optim, args.lr, args.clip, lr_decay=args.weight_decay
    )

    # 在任何时刻都可以按下Ctrl + C来提前停止训练
    try:
        print('begin training')
        for epoch in range(1, args.epochs + 1):
            epoch_start_time = time.time()
            train_loss = train(Data, Data.train[0], Data.train[1], model, criterion, optim, args.batch_size)
            val_loss, val_rae, val_corr = evaluate(Data, Data.valid[0], Data.valid[1], model, evaluateL2, evaluateL1, args.batch_size)
            print('| end of epoch {:3d} | time: {:5.2f}s | train_loss {:5.4f} | valid rse {:5.4f} | valid rae {:5.4f} | valid corr  {:5.4f}'.format(
                    epoch, (time.time() - epoch_start_time), train_loss, val_loss, val_rae, val_corr), flush=True)
            # 如果损失是目前为止看到过的最好的，请及时保存模型
            if val_loss < best_val:
                with open(args.save, 'wb') as f:
                    torch.save(model, f)
                best_val = val_loss
            if epoch % 5 == 0:
                test_acc, test_rae, test_corr = evaluate(Data, Data.test[0], Data.test[1], model, evaluateL2, evaluateL1, args.batch_size)
                print("test rse {:5.4f} | test rae {:5.4f} | test corr {:5.4f}".format(test_acc, test_rae, test_corr), flush=True)

    except KeyboardInterrupt:
        print('-' * 89)
        print('Exiting from training early')

    # 加载保存最好的模型
    with open(args.save, 'rb') as f:
        model = torch.load(f)

    vtest_acc, vtest_rae, vtest_corr = evaluate(Data, Data.valid[0], Data.valid[1], model, evaluateL2, evaluateL1, args.batch_size)
    test_acc, test_rae, test_corr = evaluate(Data, Data.test[0], Data.test[1], model, evaluateL2, evaluateL1, args.batch_size)
    print("final test rse {:5.4f} | test rae {:5.4f} | test corr {:5.4f}".format(test_acc, test_rae, test_corr))
    return vtest_acc, vtest_rae, vtest_corr, test_acc, test_rae, test_corr

if __name__ == "__main__":
    vacc = []
    vrae = []
    vcorr = []
    acc = []
    rae = []
    corr = []
    for i in range(10):
        val_acc, val_rae, val_corr, test_acc, test_rae, test_corr = main()
        vacc.append(val_acc)
        vrae.append(val_rae)
        vcorr.append(val_corr)
        acc.append(test_acc)
        rae.append(test_rae)
        corr.append(test_corr)
    print('\n\n')
    print('10 runs average')
    print('\n\n')
    print("valid\trse\trae\tcorr")
    print("mean\t{:5.4f}\t{:5.4f}\t{:5.4f}".format(np.mean(vacc), np.mean(vrae), np.mean(vcorr)))
    print("std\t{:5.4f}\t{:5.4f}\t{:5.4f}".format(np.std(vacc), np.std(vrae), np.std(vcorr)))
    print('\n\n')
    print("test\trse\trae\tcorr")
    print("mean\t{:5.4f}\t{:5.4f}\t{:5.4f}".format(np.mean(acc), np.mean(rae), np.mean(corr)))
    print("std\t{:5.4f}\t{:5.4f}\t{:5.4f}".format(np.std(acc), np.std(rae), np.std(corr)))
