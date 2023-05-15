#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include "mpi.h"

// 数据范围 -1e9~1e9
#define INF (INT32_MAX >> 2)
using namespace std;

void swap(int& a, int& b) {
    int h;
    h = a;
    a = b;
    b = h;
}
// 快排
void qs(int a[], int l, int r) {
    if (r <= l)
        return;

    int i = l - 1, j = r, p = l - 1, q = r, x;
    swap(a[(l + r) / 2], a[r]);
    x = a[r];
    while (1) {
        while (a[++i] < x)
            ;
        while (a[--j] > x)
            if (j == l)
                break;
        if (i >= j)
            break;
        swap(a[i], a[j]);
        if (a[i] == x)
            swap(a[++p], a[i]);
        if (a[j] == x)
            swap(a[--q], a[j]);
    }
    swap(a[i], a[r]);
    j = i - 1;
    i = i + 1;
    for (int k = l; k <= p; k++, j--)
        swap(a[k], a[j]);
    for (int k = r - 1; k >= q; k--, i++)
        swap(a[k], a[i]);

    qs(a, l, j);
    qs(a, i, r);
}

void rand_arr(int arr_rand[], int len, int begin, int end) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < len; i++) {
        arr_rand[i] = rand() % (begin - end + 1) + begin;
    }
}
// 归并排序
void merge(int a[], int l, int m, int r, int tmp_a[]) {
    if(r<=m){
        return;
    }
    int i, j;
    for (i = m + 1; i > l; i--) {
        tmp_a[i - 1] = a[i - 1];
    }
    for (j = m; j < r; j++) {
        tmp_a[r + m - j] = a[j + 1];
    }
    for (int k = l; k <= r; k++) {
        if (tmp_a[j] < tmp_a[i]) {
            a[k] = tmp_a[j--];
        } else {
            a[k] = tmp_a[i++];
        }
    }
}
void MergeSortBU(int a[], int l, int r,int rcounts[],int block_num) {
    int* tmp_a = (int*)malloc(sizeof(int) * (r+1));
    int index=1;
    int m;
    while(index<block_num){
        m=l+rcounts[index-1]-1;
        r=m+rcounts[index];
        merge(a, l, m, r, tmp_a);
        rcounts[index]+=rcounts[index-1];
        index++;
    }
    free(tmp_a);
}

int main(int argc, char* argv[]) {
    int *array, *a;
    int* result;
    int n;
    int num_procs, id_procs;
    int group, group_len;
    int mod;

    int i, index;

     double run_time;
    struct timeval start, end;
    // 开始mpi
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_procs);

    if (argc != 2) {
        printf("请输入数组大小\n");
        return 1;
    }
    n = atoi(argv[1]);
    if (n <= 0) {
        printf("请输入正整数\n");
        return 1;
    }

    group = n / num_procs;
    mod = n % num_procs;
    a = (int*)malloc((group + mod) * sizeof(int));

    if (id_procs == 0) {
        // 产生随机数据
        // 串行运行 得出结果
        array = (int*)malloc((n + 2) * sizeof(int));
        result = (int*)malloc((n + 2) * sizeof(int));
        rand_arr(array, n, -INF, INF);
         gettimeofday(&start, NULL);
    }

    // 分段数据 移交各个处理器
    int sendCounts[num_procs];
    int s_displs[num_procs];
    for (int i = 0; i < num_procs; i++) {
        sendCounts[i] = group;
        s_displs[i] = i * group;
    }
    sendCounts[num_procs - 1] = group + mod;

    MPI_Scatterv(array, sendCounts, s_displs, MPI_INT, a, group + mod, MPI_INT,
                 0, MPI_COMM_WORLD);

    group_len = sendCounts[id_procs];
    // 均匀划分 局部排序,快排
    qs(a, 0, group_len - 1);
    // 正则采样,采样 p-1个
    int samples[num_procs * num_procs];
    int s[num_procs];
    
    for (i = 1; i < num_procs; i++) {
        s[i - 1] = a[i * group_len / num_procs];
       
    }
 
    // 采到样本 收集到Proc#0
    MPI_Gather(s, num_procs - 1, MPI_INT, samples, num_procs - 1, MPI_INT, 0,
               MPI_COMM_WORLD);

    // 采样排序
    int pivot[num_procs];
    if (id_procs == 0) {
        qs(samples, 0, (num_procs - 1) * num_procs - 1);
        for (i = 1; i < num_procs; i++)
            pivot[i - 1] = samples[i * (num_procs - 1)];
    }
    // 广播主元
    MPI_Bcast(pivot, num_procs - 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 进行主元划分
    index = 0;
    int pcounts[num_procs];
    memset(pcounts, 0, num_procs * sizeof(int));
    pivot[num_procs - 1] = INT32_MAX;

    for (i = 0; i < group_len;) {
        if (a[i] <= pivot[index]) {
            pcounts[index]++;
            i++;
        } else {
            index++;
        }
    }

    // 发送各个段的长度
    int rcounts[num_procs];
    MPI_Alltoall(pcounts, 1, MPI_INT, rcounts, 1, MPI_INT, MPI_COMM_WORLD);

    // 计算位移
    int rdispls[num_procs];
    s_displs[0] = 0;
    rdispls[0] = 0;
    for (i = 1; i < num_procs; i++) {
        s_displs[i] = s_displs[i - 1] + pcounts[i - 1];
        rdispls[i] = rdispls[i - 1] + rcounts[i - 1];
    }
    // 计算总长度
    int totalcounts = rdispls[num_procs - 1] + rcounts[num_procs - 1];

    int* b = (int*)malloc(totalcounts * sizeof(int));
    // 每个处理器发送数据给其他所有处理器，且每个处理发送的数据长度都不同
    // 故有长度数组和位移数组
    MPI_Alltoallv(a, pcounts, s_displs, MPI_INT, b, rcounts, rdispls, MPI_INT,
                  MPI_COMM_WORLD);

    // 归并排序，考虑清楚步长
    MergeSortBU(b, 0, totalcounts - 1,rcounts,num_procs);

    // Proc#0 收集有序数组
    MPI_Gather(&totalcounts, 1, MPI_INT, rcounts, 1, MPI_INT, 0,
               MPI_COMM_WORLD);

    rdispls[0] = 0;
    for (i = 1; i < num_procs; i++)
        rdispls[i] = rdispls[i - 1] + rcounts[i - 1];

    MPI_Gatherv(b, totalcounts, MPI_INT, result, rcounts, rdispls, MPI_INT, 0,
                MPI_COMM_WORLD);
     if (id_procs == 0) {
      gettimeofday(&end, NULL);
    run_time = (double)(int)(end.tv_sec - start.tv_sec) * 1000 +
               (double)(end.tv_usec - start.tv_usec) / 1000;
      printf("%dK 并行 %d进程程 运行时间为:  %f ms \n", n / 1000, num_procs,
           run_time);
     }
    if (id_procs == 0) {
        free(result);
        free(array);
    }
    free(b);
    free(a);

    MPI_Finalize();
    return 0;
}