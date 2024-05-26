#include <immintrin.h>
#include <omp.h>
#include <windows.h>
#include <iostream>
#define NUM_THREADS 4
#define N 1024
using namespace std;

float matrix[N][N];

void createMatrix() {
    srand(20000214);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 100;
            // cout << matrix[i][j] << " ";
        }
        // cout << endl;
    }
    // cout << endl;
}

void LU() {
    for (int k = 0; k < N; k++) {
        for (int j = k + 1; j < N; j++) {
            matrix[k][j] /= matrix[k][k];
        }
        matrix[k][k] = 1;
        for (int i = k + 1; i < N; i++) {
            for (int j = k + 1; j < N; j++) {
                matrix[i][j] -= matrix[i][k] * matrix[k][j];
            }
            matrix[i][k] = 0;
        }
    }
}

void openmp_col() {
    for (int k = 0; k < N; k++) {
#pragma omp parallel num_threads(NUM_THREADS)
        {
            int len = (N - k) / NUM_THREADS;
            int start, end;
            int threadId = omp_get_thread_num();
            start = k + threadId * len;
            if (start == k) {
                start++;
            }
            if (threadId != NUM_THREADS - 1) {
                end = k + (threadId + 1) * len;
            } else {
                end = N;
            }
            for (int j = start; j < end; j++) {
                matrix[k][j] /= matrix[k][k];
            }
            for (int i = k + 1; i < N; i++) {
                for (int j = start; j < end; j++) {
                    matrix[i][j] -= matrix[i][k] * matrix[k][j];
                }
            }
        }
        matrix[k][k] = 1;
        for (int i = k + 1; i < N; i++) {
            matrix[i][k] = 0;
        }
    }
}

void openmp_row() {
    for (int k = 0; k < N; k++) {
        for (int j = k + 1; j < N; j++) {
            matrix[k][j] /= matrix[k][k];
        }
        matrix[k][k] = 1;
#pragma omp parallel num_threads(NUM_THREADS)
        {
            int threadId = omp_get_thread_num();
            int len = (N - k - 1) / NUM_THREADS;
            int start = k + 1 + len * threadId, end;
            if (threadId != NUM_THREADS - 1) {
                end = k + 1 + (threadId + 1) * len;
            } else {
                end = N;
            }
            for (int i = start; i < end; i++) {
                for (int j = k + 1; j < N; j++) {
                    matrix[i][j] -= matrix[i][k] * matrix[k][j];
                }
                matrix[i][k] = 0;
            }
        }
    }
}

void openmp_row_sse() {
    for (int k = 0; k < N; k++) {
        for (int j = k + 1; j < N; j++) {
            matrix[k][j] /= matrix[k][k];
        }
        matrix[k][k] = 1;
#pragma omp parallel num_threads(NUM_THREADS)
        {
            int threadId = omp_get_thread_num();
            int len = (N - k - 1) / NUM_THREADS;
            int start = k + 1 + len * threadId, end;
            if (threadId != NUM_THREADS - 1) {
                end = k + 1 + (threadId + 1) * len;
            } else {
                end = N;
            }
            __m128 t1, t2, t3, t4;
            for (int i = start; i < end; i++) {
                float ik[4] = {matrix[i][k], matrix[i][k], matrix[i][k],
                               matrix[i][k]};
                t1 = _mm_loadu_ps(ik);
                for (int j = N - 4; j > k; j -= 4) {
                    t2 = _mm_loadu_ps(matrix[i] + j);
                    t3 = _mm_loadu_ps(matrix[k] + j);
                    t4 = _mm_sub_ps(t2, _mm_mul_ps(t1, t3));
                    _mm_storeu_ps(matrix[i] + j, t4);
                }
                for (int j = k + 1; j % 4 != N % 4; j++) {
                    matrix[i][j] -= matrix[i][k] * matrix[k][j];
                }
                matrix[i][k] = 0;
            }
        }
    }
}

int main() {
    createMatrix();
    // long freq, head, tail;
    // QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    // QueryPerformanceCounter((LARGE_INTEGER*)&head);
    // openmp_row();
    // QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    // cout << "openmp row: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    // createMatrix();
    // QueryPerformanceCounter((LARGE_INTEGER*)&head);
    // openmp_col();
    // QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    // cout << "openmp col: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    // createMatrix();
    // QueryPerformanceCounter((LARGE_INTEGER*)&head);
    openmp_row_sse();
    // QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    // cout << "openmp row sse: " << (tail - head) * 1000.0 / freq << "ms" <<
    // endl;
}
