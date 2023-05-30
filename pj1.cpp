#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
// 数据范围 -1e9~1e9
#define INF INT32_MAX / 2

using namespace std;
int* tmp_a;
void swap(int& a, int& b) {
    int h;
    h = a;
    a = b;
    b = h;
}
// 生成随机数组
void rand_arr(int arr_rand[], int len, int begin, int end) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < len; i++) {
        arr_rand[i] = rand() % (begin - end + 1) + begin;
    }
}
// 归并排序
void merge(int a[], int l, int m, int r) {
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
void MergeSortBU(int a[], int l, int r, int step) {
    for (int m = step; m < r; m += m) {
// 进行并行的merge
#pragma omp parallel for
        for (int i = l; i <= r - m; i += m + m) {
            merge(a, i, i + m - 1, min(i + m + m - 1, r));
        }
    }
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

void parallel_quick_sort(int a[], int n, int thread_num) {
    int step = ceil((double)n / thread_num);

#pragma omp parallel
    {
        int index;
#ifdef _OPENMP
        index = omp_get_thread_num();
#endif
        int l, r;
        l = index * step;
        r = min(l + step - 1, n - 1);
        // printf("%d  %d   %d\n", index, l, r);
        qs(a, l, r);
    }
    if(thread_num>1){
        MergeSortBU(a, 0, n - 1, step);
    }
}

double test(int n, int m) {
    double run_time;
    int* init_array;
    init_array = (int*)malloc(sizeof(int) * (n + 5));
#ifdef _OPENMP
    // 设置线程数
    omp_set_num_threads(m);
    tmp_a = (int*)malloc(sizeof(int) * (n + 5));
#endif
    rand_arr(init_array, n, -INF, INF);

    struct timeval start, end;
    gettimeofday(&start, NULL);

#ifdef _OPENMP
    parallel_quick_sort(init_array, n, m);
#else
    qs(init_array, 0, n - 1);
#endif

    gettimeofday(&end, NULL);
    run_time = (double)(int)(end.tv_sec - start.tv_sec) * 1000 +
               (double)(end.tv_usec - start.tv_usec) / 1000;
// printf("线程数： %d\n",omp_get_num_threads());
#ifdef _OPENMP
    // printf("%dK 并行 %d线程 运行时间为:  %f ms \n", n / 1000, m,
    //        run_time);
    printf("%f ", run_time);
    free(tmp_a);
#else
    printf("%dK 串行 运行时间为:  %f ms\n", n / 1000, run_time);
#endif
    // for (int i = 0; i < n; i++)
    // 	printf("%d\n", init_array[i]);
    // for (int i = 1; i < n; i++){
    //     if(init_array[i]<init_array[i-1]){
    //         printf("错误 %d\n",i);
    //         break;
    //     }
    // }
    free(init_array);
    return run_time;
}
int main(int argc, char* argv[]) {
    int n, m;
    // 设置数组规模
    // n = atoi(argv[1]);
    // m = atoi(argv[2]);
    double single_time[10];
    int x[] = {1000, 5000, 10000, 100000, 1000000, 10000000};
    double runtime[10];
#ifdef _OPENMP
    for (int i = 0; i < 6; i++) {
        printf("%dK\n",x[i]/1000);
        for (int m = 1,j=0; m <= 32; m *= 2,j++) {
            runtime[j]=test(x[i], m);
        }
        printf("\n");
        for(int j=0;j<6;j++){
            printf("%f ",runtime[0]/runtime[j]);
        }
        printf("\n");
    }
#else
    for (int i = 0; i < 6; i++) {
        test(x[i], 1);
    }
#endif

    return 0;
}