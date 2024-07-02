#include <stdbool.h>
#include <windows.h>
#include "gaussian.c"

int main() {
    // 设置矩阵大小与sigma
    uint32_t gaussianSize = 7;
    float gaussianSigma = 7 / 3;
    char* imgname = "../2.bmp";

    long freq, head, tail;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);
    if (blur_cpu(imgname, gaussianSize, gaussianSigma) == false)
        return -1;
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    printf("cpu: %lfms\n", (tail - head) * 1000.0 / freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);
    if (blur_gpu(imgname, gaussianSize, gaussianSigma) == false)
        return -2;
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    printf("gpu: %lfms\n", (tail - head) * 1000.0 / freq);
    return 0;
}
