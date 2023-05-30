#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <cuda.h>
#include "PJ3.h"

#define blockSize 32

template <int BLOCK_SIZE> __global__ void MatMulKernel2DAnySize(double* C, double* A, double* B, int wA, int wC, int hC)
{
    int wB = wC;
    int maxIdxA = wA * hC;

    // Block index
    int bx = blockIdx.x;
    int by = blockIdx.y;
    // Thread index
    int tx = threadIdx.x;
    int ty = threadIdx.y;

    while (wA * BLOCK_SIZE * by < maxIdxA) {
        // Index of the first sub-matrix of A processed by the block
        int aBegin = wA * BLOCK_SIZE * by;

        // Index of the last sub-matrix of A processed by the block
        int aEnd = aBegin + wA - 1;

        // Step size used to iterate through the sub-matrices of A
        int aStep = BLOCK_SIZE;

        // Index of the first sub-matrix of B processed by the block
        int bBegin = BLOCK_SIZE * bx;

        // Step size used to iterate through the sub-matrices of B
        int bStep = BLOCK_SIZE * wB;

        // Csub is used to store the element of the block sub-matrix
        // that is computed by the thread
        double Csub = 0;

        // Loop over all the sub-matrices of A and B
        // required to compute the block sub-matrix
        int flag = 0;
        for (int a = aBegin, b = bBegin; a <= aEnd; a += aStep, b += bStep) {
            // Declaration of the shared memory array As used to
            // store the sub-matrix of A
            __shared__ double As[BLOCK_SIZE][BLOCK_SIZE];

            // Declaration of the shared memory array Bs used to
            // store the sub-matrix of B
            __shared__ double Bs[BLOCK_SIZE][BLOCK_SIZE];

            // Load the matrices from device memory
            // to shared memory; each thread loads
            // one element of each matrix
            if (flag * BLOCK_SIZE + tx < wA || flag * BLOCK_SIZE + ty < hC) {
                As[ty][tx] = A[a + wA * ty + tx];
            }
            else {
                As[ty][tx] = 0.0;
            }

            if (flag * BLOCK_SIZE + ty < wA || flag * BLOCK_SIZE + tx < wC) {
                Bs[ty][tx] = B[b + wB * ty + tx];
            }
            else {
                Bs[ty][tx] = 0.0;
            }

            // Synchronize to make sure the matrices are loaded
            __syncthreads();

            // Multiply the two matrices together;
            // each thread computes one element
            // of the block sub-matrix
#pragma unroll

            for (int k = 0; k < BLOCK_SIZE; ++k) {
                Csub += As[ty][k] * Bs[k][tx];
            }

            // Synchronize to make sure that the preceding
            // computation is done before loading two new
            // sub-matrices of A and B in the next iteration
            __syncthreads();
            flag++;
        }

        // Write the block sub-matrix to device memory;
        // each thread writes one element
        if (BLOCK_SIZE * bx + tx < wC && BLOCK_SIZE * by + ty < hC) { // thread could over max.
            C[wB * BLOCK_SIZE * by + BLOCK_SIZE * bx + wB * ty + tx] = Csub;
        }
        bx += BLOCK_SIZE;
        by += BLOCK_SIZE;
    }
}

void Matrix_Mul(int N, double h_A[],double h_B[],double h_C[]){
    
    // Allocate device memory
    double* d_A, * d_B, * d_C;

    // Allocate host matrix C
    unsigned int mem_size_C = N * N * sizeof(double);

    if (h_C == NULL) {
        fprintf(stderr, "Failed to allocate host matrix C!\n");
        exit(EXIT_FAILURE);
    }

    cudaMalloc(reinterpret_cast<void**>(&d_A), mem_size_C);
    cudaMalloc(reinterpret_cast<void**>(&d_B), mem_size_C);
    cudaMalloc(reinterpret_cast<void**>(&d_C), mem_size_C);
 

    // copy host memory to device
    cudaMemcpy(d_A, h_A, mem_size_C, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, mem_size_C, cudaMemcpyHostToDevice);

    // Setup execution parameters
    dim3 threads(blockSize, blockSize);
    dim3 grid;

    // select diff blocks for kerenl
    void (*MMKernel2DAnySizeExe)(double*, double*, double*, int, int, int);
    if (blockSize <= 16) {
        MMKernel2DAnySizeExe = MatMulKernel2DAnySize<16>;
    }
    else {
        MMKernel2DAnySizeExe = MatMulKernel2DAnySize<32>;
    }

    grid = dim3(N / threads.x+1, N / threads.y+1);
    MMKernel2DAnySizeExe << <grid, threads>> > (d_C, d_A, d_B, N, N, N);

    // Copy result from device to host
    cudaMemcpy(h_C, d_C, mem_size_C, cudaMemcpyDeviceToHost);

    // Clean up memory
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
}