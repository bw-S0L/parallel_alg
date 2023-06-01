import numpy as np
import matplotlib.pyplot as plt
import matplotlib

def CPU():
    y1 =[47, 422, 14515, 154219]
    y2 = [15, 109, 1547, 37203]
    y3 = [16, 47, 453, 4813]

    for i in range(0,4):
        y2[i]=y1[i]/y2[i]
        y3[i]=y1[i]/y3[i]
        y1[i]=y1[i]/y1[i]

    matplotlib.rc("font", family='MicroSoft YaHei')

    length = len(y1)
    x = np.arange(length)   # 横坐标范围
    listDate = [256, 512, 1024, 2048]

    plt.figure()
    total_width, n = 0.8, 3   # 柱状图总宽度，有几组数据
    width = total_width / n   # 单个柱状图的宽度
    x1 = x - width # 第一组数据柱状图横坐标起始位置
    x2 = x1 + width   # 第二组数据柱状图横坐标起始位置
    x3=x2+width

    # plt.xlabel("星期")   # 横坐标label 此处可以不添加
    plt.ylabel("加速比")   # 纵坐标label
    plt.bar(x1, y1, width=width, label="1  进程",color='deepskyblue',alpha=0.6)
    plt.bar(x2, y2, width=width, label="4  进程",color='yellow',alpha=0.6)
    plt.bar(x3, y3, width=width, label="16 进程",color='coral',alpha=0.6)
    plt.xticks(x, listDate)   # 用星期几替换横坐标x的值
    for a, b in zip(x1, y1):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)

    for a, b in zip(x2, y2):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)
    for a, b in zip(x3, y3):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)
    plt.legend()   # 给出图例
    plt.show()


def Cuda():


    y1 =[94, 125, 172, 687,7875,103282]
    y2 = [500, 328, 375, 750,5187,61016]
    y3 = [1454, 1328, 1578, 1890,5610,40656]

    for i in range(0,6):
        y2[i]=y1[i]/y2[i]
        y3[i]=y1[i]/y3[i]
        y1[i]=y1[i]/y1[i]

    matplotlib.rc("font", family='MicroSoft YaHei')

    length = len(y1)
    x = np.arange(length)   # 横坐标范围
    listDate = [256, 512, 1024, 2048,4096,8192]

    plt.figure()
    total_width, n = 0.8, 3   # 柱状图总宽度，有几组数据
    width = total_width / n   # 单个柱状图的宽度
    x1 = x - width # 第一组数据柱状图横坐标起始位置
    x2 = x1 + width   # 第二组数据柱状图横坐标起始位置
    x3=x2+width

    # plt.xlabel("星期")   # 横坐标label 此处可以不添加
    plt.ylabel("加速比")   # 纵坐标label
    plt.bar(x1, y1, width=width, label="1  进程",color='deepskyblue',alpha=0.6)
    plt.bar(x2, y2, width=width, label="4  进程",color='yellow',alpha=0.6)
    plt.bar(x3, y3, width=width, label="16 进程",color='coral',alpha=0.6)
    plt.xticks(x, listDate)   # 用星期几替换横坐标x的值
    for a, b in zip(x1, y1):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)

    for a, b in zip(x2, y2):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)
    for a, b in zip(x3, y3):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)
    plt.legend()   # 给出图例
    plt.show()

def CPU_Cuda():
    c_y1 = [47, 422, 14515, 154219]
    c_y2 = [15, 109, 1547, 37203]
    c_y3 = [16, 47, 453, 4813]

    y1 =[94, 125, 172, 687]
    y2 = [500, 328, 375, 750]
    y3 = [1454, 1328, 1578, 1890]

    for i in range(0,4):
        y2[i]=c_y1[i]/y2[i]
        y3[i]=c_y1[i]/y3[i]
        y1[i]=c_y1[i]/y1[i]
    # for i in range(0,4):
    #     y2[i]=c_y2[i]/y2[i]
    #     y3[i]=c_y3[i]/y3[i]
    #     y1[i]=c_y1[i]/y1[i]
    matplotlib.rc("font", family='MicroSoft YaHei')

    length = len(y1)
    x = np.arange(length)   # 横坐标范围
    listDate = [256, 512, 1024, 2048]

    plt.figure()
    total_width, n = 0.8, 3   # 柱状图总宽度，有几组数据
    width = total_width / n   # 单个柱状图的宽度
    x1 = x - width # 第一组数据柱状图横坐标起始位置
    x2 = x1 + width   # 第二组数据柱状图横坐标起始位置
    x3=x2+width

    # plt.xlabel("星期")   # 横坐标label 此处可以不添加
    plt.ylabel("加速比")   # 纵坐标label
    plt.bar(x1, y1, width=width, label="1  进程",color='deepskyblue',alpha=0.6)
    plt.bar(x2, y2, width=width, label="4  进程",color='yellow',alpha=0.6)
    plt.bar(x3, y3, width=width, label="16 进程",color='coral',alpha=0.6)
    plt.xticks(x, listDate)   # 用星期几替换横坐标x的值
    for a, b in zip(x1, y1):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)

    for a, b in zip(x2, y2):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)
    for a, b in zip(x3, y3):
        plt.text(a, b + 0.1, '%.2f' % b, ha='center', va='bottom', fontsize=7)
    plt.legend()   # 给出图例
    plt.show()

CPU_Cuda()
