#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
// kernel源码
const char* programSource =
    "__kernel                                         \n"
    "void vecadd(__global int *A,                     \n"
    "            __global int *B,                     \n"
    "            __global int *C)                     \n"
    "{                                                \n"
    "  int idx = get_global_id(0);                    \n"
    "  C[idx] = A[idx] + B[idx];                      \n"
    "}                                                \n";
int main() {
    const int elements = 2048;
    size_t datasize = sizeof(int) * elements;
    int* A = (int*)malloc(datasize);
    int* B = (int*)malloc(datasize);
    int* C = (int*)malloc(datasize);
    int i;
    for (i = 0; i < elements; i++) {
        A[i] = i;
        B[i] = i;
    }
    cl_int status;            // 用于错误检查
    cl_platform_id platform;  // 取1号平台
    status = clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;  // 获取设备
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
    // 创建一个上下文，包含所有找到的设备
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
    // 为设备创建命令队列
    cl_command_queue cmdQueue =
        clCreateCommandQueueWithProperties(context, device, 0, &status);
    // 向量加法的三个向量，2个输入数组和1个输出数组
    cl_mem bufA =
        clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, &status);
    cl_mem bufB =
        clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, &status);
    cl_mem bufC =
        clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize, NULL, &status);
    // 将输入数据填充到数组中
    status = clEnqueueWriteBuffer(cmdQueue, bufA, CL_FALSE, 0, datasize, A, 0,
                                  NULL, NULL);
    status = clEnqueueWriteBuffer(cmdQueue, bufB, CL_FALSE, 0, datasize, B, 0,
                                  NULL, NULL);
    // 使用源码创建程序
    cl_program program = clCreateProgramWithSource(
        context, 1, (const char**)&programSource, NULL, &status);
    // 为设备构建(编译)程序
    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    // 创建向量相加内核
    cl_kernel kernel = clCreateKernel(program, "vecadd", &status);
    // 设置内核参数
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufB);
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufC);
    // 定义工作项的空间维度和空间大小
    size_t indexSpaceSize[1], workGroupSize[1];
    indexSpaceSize[0] = elements;
    workGroupSize[0] = 256;
    // 通过执行API执行内核
    status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, indexSpaceSize,
                                    workGroupSize, 0, NULL, NULL);
    // 将输出数组拷贝到主机端内存中
    status = clEnqueueReadBuffer(cmdQueue, bufC, CL_TRUE, 0, datasize, C, 0,
                                 NULL, NULL);
    // 释放资源
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    clReleaseMemObject(bufC);
    clReleaseContext(context);
    free(A);
    free(B);
    free(C);
    return 0;
}
