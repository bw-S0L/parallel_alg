#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<time.h>
#include<omp.h>

//用wsl环境，但coding环境是windows，避免飘红
#ifdef __linux__
#include <sys/time.h> 
#include <unistd.h>
#endif


//最大规模 100K
#define N 1000005
//数据范围 -1e9~1e9
#define INF INT32_MAX/2
int init_array[N];
using namespace std;

void swap(int& a, int& b) {
	int h;
	h = a;
	a = b;
	b = h;
}
//生成随机数组
void rand_arr(int arr_rand[], int len, int begin, int end){
    srand((unsigned)time(NULL));
    for (int i = 0; i < len; i++){
        arr_rand[i] = rand() % (begin - end + 1) + begin;
    }
}
void qs(int a[], int l, int r) {
	if (r <= l) return;
	
	int i = l-1, j = r,p=l-1,q=r,x;
	swap(a[(l + r) / 2], a[r]);
	x = a[r];
	while (1) {
		while (a[++i] < x);
		while (a[--j] > x)
			if (j == l)
				break;
		if (i >= j) break;
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
	for (int k = r-1; k >= q; k--, i++)
		swap(a[k], a[i]);
    
#pragma omp parallel  sections//并行区域
    {  
#pragma omp section
	{	
		printf("1： %d\n",omp_get_thread_num());
        qs(a, l, j);
	}
#pragma omp section
	{	
		printf("2： %d\n",omp_get_thread_num());
        qs(a, i, r);   
	}
    }
}
int main(int argc,char* argv[]) {
	int n;
    double run_time;
	n=atoi(argv[1]);
    rand_arr(init_array,n,-INF,INF);

#ifdef __linux__
    struct timeval start, end;
    gettimeofday( &start, NULL );
#endif
    qs(init_array, 0, n-1);	
#ifdef __linux__
    gettimeofday( &end, NULL );
#endif
#ifdef __linux__
	run_time=(double)(int)(end.tv_sec-start.tv_sec)*1000+(double)(end.tv_usec- start.tv_usec)/1000;
	printf("线程数： %d\n",omp_get_num_threads());
    printf("%dK 运行时间为:\n%f ms\n",n/1000,run_time);
#endif
	// for (int i = 0; i < n; i++)
	// 	printf("%d\n", init_array[i]);
    // printf("\n");
	return 0;
}