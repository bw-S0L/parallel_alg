#include "iostream"
#include "cstdlib"
#include "cstring"
#include "mpi.h"
#include "ctime"
#include "cmath"
#include "algorithm"
#include "vector"
#include <time.h>
#include <windows.h>
#include "PJ3.h"
using namespace std;

// 数据范围,应该避免溢出
#define INF (INT32_MAX >> 12)
//#define MY_COMPUTE_SELECT 
/**************** 全局变量声明**************/
//矩阵大小
int N;
//block是块的大小，block_size是块元素个数，等于block*block，num_procs是处理器个数，dim是sqrt(num_procs)
int block, block_size, num_procs, dim;
double** A, ** B, ** C;
double* a, * b, * c, * temp_a, * temp_b;
// id_procs是处理器的id，(my_row, my_col)是处理器逻辑阵列坐标
int id_procs, my_row, my_col;
MPI_Status status;

double run_time;
time_t start_time, end_time;

//处理器逻辑阵列坐标到rank的转换
int get_index(int row, int col, int sp);
//随机生成矩阵A和B
void init_Matrix();
//id为0的处理器向其他处理器发送矩阵A、B的分块
void scatter_A_B();
//矩阵A和B的初始化，即A每行不同程度的左移，B每行不同程度的上移
void init_alignment();
//矩阵A每行左移一位，矩阵B每行上移一位，同时计算分块的中间结果C
void shift();
//id为0的处理器从其余处理器收集分块矩阵C
void collect_C();
void compute_CPU();

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_procs);
    dim = sqrt(num_procs);

    if (dim * dim != num_procs) {
        if (id_procs == 0) {
            printf("进程数 %d 不是平方数\n", num_procs);
        }
        MPI_Finalize();
        exit(1);
    }
    N = atoi(argv[1]);
    block = N / dim;
    block_size = block * block;

    my_col = id_procs % dim;
    my_row = id_procs / dim;

    a = (double*)malloc(block_size * sizeof(double));
    b = (double*)malloc(block_size * sizeof(double));
    c = (double*)malloc(block_size * sizeof(double));;

    for (int i = 0; i < block_size; ++i) {
        c[i] = 0;
    }
    temp_a = (double*)malloc(block_size * sizeof(double));
    temp_b = (double*)malloc(block_size * sizeof(double));

    if (id_procs == 0) {
        init_Matrix();
        start_time = GetTickCount();
        scatter_A_B();
    }
    else {
        MPI_Recv(a, block_size, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(b, block_size, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &status);
    }
    init_alignment();
    shift();

    if (id_procs == 0) {
        collect_C();
    }
    else {
        MPI_Send(c, block_size, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
    }
    if (id_procs == 0) {
        end_time = GetTickCount();;
        run_time = double(end_time - start_time);
#ifdef MY_COMPUTE_SELECT 
        printf("CPU: Dim: %d Parral  Processes: %d RunTime: %f ms \n", N, num_procs, run_time);
#else
        printf("GPU: Dim: %d Parral  Processes: %d RunTime: %f ms \n", N, num_procs, run_time);
#endif
        
    }
    MPI_Finalize();

    return 0;
}

/*
 处理器逻辑阵列坐标到rank的转换
 输入：坐标(row, col)，sqrt(p) processes
 输出：rank
 */
int get_index(int row, int col, int dim) {
    return ((row + dim) % dim) * dim + (col + dim) % dim;
}

/*
分配空间
 随机生成矩阵A和B
 */
void init_Matrix() {
    A = (double**)malloc(N * sizeof(double));
    B = (double**)malloc(N * sizeof(double));
    C = (double**)malloc(N * sizeof(double));
    srand((unsigned int)time(NULL)); //设置随机数种子
    for (int i = 0; i < N; i++) {
        A[i] = (double*)malloc(N * sizeof(double));
        B[i] = (double*)malloc(N * sizeof(double));
        C[i] = (double*)malloc(N * sizeof(double));
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % INF;
            B[i][j] = rand() % INF;
            C[i][j] = 0;
        }
    }
}

/*
 id为0的处理器向其他处理器发送矩阵A、B的分块
 */
void scatter_A_B() {
    int p_i_min, p_j_min, p_i_max, p_j_max;
    for (int k = 0; k < num_procs; k++) {
        p_j_min = (k % dim) * block;
        p_j_max = (k % dim + 1) * block - 1;
        p_i_min = k / dim * block;
        p_i_max = (k / dim + 1) * block - 1;
        // id为0的处理器将矩阵A和B中的数据转换成一位数组传到temp中
        for (int i = p_i_min; i <= p_i_max; ++i) {
            for (int j = p_j_min; j <= p_j_max; j++) {
                temp_a[(i - p_i_min) * block + j - p_j_min] = A[i][j];
                temp_b[(i - p_i_min) * block + j - p_j_min] = B[i][j];
            }
        }
        if (k == 0) { // id为0的处理器直接拷贝过去，其他处理器则发送过去
            memcpy(a, temp_a, block_size * sizeof(double));
            memcpy(b, temp_b, block_size * sizeof(double));
        }
        else {
            MPI_Send(temp_a, block_size, MPI_FLOAT, k, 1, MPI_COMM_WORLD);
            MPI_Send(temp_b, block_size, MPI_FLOAT, k, 2, MPI_COMM_WORLD);
        }
    }
}

/*
 将A中坐标为(i, j)的分块循环左移i步
 将B中坐标为(i, j)的分块循环上移j步
 */
void init_alignment() {
    MPI_Sendrecv(a, block_size, MPI_FLOAT, get_index(my_row, my_col - my_row, dim), 1, temp_a, block_size, MPI_FLOAT,
        get_index(my_row, my_col + my_row, dim), 1, MPI_COMM_WORLD, &status);
    memcpy(a, temp_a, block_size * sizeof(double));
    MPI_Sendrecv(b, block_size, MPI_FLOAT, get_index(my_row - my_col, my_col, dim), 1, temp_b, block_size, MPI_FLOAT,
        get_index(my_row + my_col, my_col, dim), 1, MPI_COMM_WORLD, &status);
    memcpy(b, temp_b, block_size * sizeof(double));
}


void compute_CPU() {
    double total = 0;
    for (int i = 0; i < block; i++) {
        for (int j = 0; j < block; j++) {
            total = c[i * block + j];
            for (int k = 0; k < block; k++) {
                total += a[i * block + k] * b[k * block + j];
            }
            c[i * block + j] = total;
        }

    }
}
/*
 矩阵A每行左移一位，矩阵B每行上移一位，同时计算分块的中间结果C
 */
void shift() {
    for (int l = 0; l < dim; l++) {
        //选择计算方式
#ifdef MY_COMPUTE_SELECT 
        compute_CPU();
#else
        Matrix_Mul(block,a,b,c);
#endif
        // 将A全部循环左移1位
        MPI_Sendrecv(a, block_size, MPI_FLOAT, get_index(my_row, my_col - 1, dim), 1, temp_a, block_size, MPI_FLOAT,
            get_index(my_row, my_col + 1, dim), 1, MPI_COMM_WORLD, &status);
        memcpy(a, temp_a, block_size * sizeof(double));
        // 将B全部循环上移1位
        MPI_Sendrecv(b, block_size, MPI_FLOAT, get_index(my_row - 1, my_col, dim), 1, temp_b, block_size, MPI_FLOAT,
            get_index(my_row + 1, my_col, dim), 1, MPI_COMM_WORLD, &status);
        memcpy(b, temp_b, block_size * sizeof(double));
    }
}

/*
 id为0的处理器从其余处理器收集分块矩阵C
 */
void collect_C() {
    int p_i_min, p_j_min, p_i_max, p_j_max;
    // 将id为0的处理器的分块矩阵c结果赋给C对应位置
    for (int i = 0; i < block; i++) {
        for (int j = 0; j < block; j++) {
            C[i][j] = c[i * block + j];
        }
    }
    for (int k = 1; k < num_procs; k++) {
        MPI_Recv(c, block_size, MPI_FLOAT, k, 1, MPI_COMM_WORLD, &status);
        p_j_min = (k % dim) * block;
        p_j_max = (k % dim + 1) * block - 1;
        p_i_min = k / dim * block;
        p_i_max = (k / dim + 1) * block - 1;
        for (int i = p_i_min; i <= p_i_max; i++) {
            for (int j = p_j_min; j <= p_j_max; ++j) {
                C[i][j] = c[(i - p_i_min) * block + j - p_j_min];
            }
        }
    }
}
