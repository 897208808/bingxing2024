#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"

#include <CL/cl.h>

#include <math.h>

#define PI_ 3.14159265359f

#define MAX_SOURCE_SIZE (1048576)
#define MAX_LOG_SIZE (1048576)
// 创建卷积核
float* createGaussianKernel(uint32_t size, float sigma) {
    float* ret;
    uint32_t x, y;
    double center = size / 2;
    float sum = 0;
    ret = malloc(sizeof(float) * size * size);
    for (x = 0; x < size; x++) {
        for (y = 0; y < size; y++) {
            ret[y * size + x] = exp((((x - center) * (x - center) +
                                      (y - center) * (y - center)) /
                                     (2.0f * sigma * sigma)) *
                                    -1.0f) /
                                (2.0f * PI_ * sigma * sigma);
            sum += ret[y * size + x];
        }
    }
    // 归一化
    for (x = 0; x < size * size; x++) {
        ret[x] = ret[x] / sum;
    }
    // 打印
    printf("The generated Gaussian Kernel is:\n");
    for (x = 0; x < size; x++) {
        for (y = 0; y < size; y++) {
            printf("%f ", ret[y * size + x]);
        }
        printf("\n");
    }
    printf("\n\n");
    return ret;
}

// cpu计算
char blur_cpu(char* imgname, uint32_t size, float sigma) {
    uint32_t i, x, y, imgLineSize;
    int32_t center, yOff, xOff;
    float *matrix, value;
    matrix = createGaussianKernel(size, sigma);
    // 利用开源实现读取BMP文件
    ImageBMP bmp;
    if (ImageBMP_Init(&bmp, imgname) == false) {
        printf("Image \"%s\" could not be read as a .BMP file\n", imgname);
        return false;
    }
    imgLineSize = bmp.imgWidth * 3;
    center = size / 2;
    // 遍历做卷积操作
    for (i = imgLineSize * (size - center) + center * 3;
         i < bmp.imgHeight * bmp.imgWidth * 3 - imgLineSize * (size - center) -
                 center * 3;
         i++) {
        value = 0;
        for (y = 0; y < size; y++) {
            yOff = imgLineSize * (y - center);
            for (x = 0; x < size; x++) {
                xOff = 3 * (x - center);
                value += matrix[y * size + x] * bmp.imgData[i + xOff + yOff];
            }
        }
        bmp.imgData[i] = value;
    }
    free(matrix);
    // 存储
    ImageBMP_Save(&bmp, "cpu_blur.bmp");
    return true;
}

// gpu计算
char blur_gpu(char* imgname, uint32_t size, float sigma) {
    uint32_t imgSize;
    float* matrix;
    cl_int ret;
    ImageBMP bmp;
    ImageBMP_Init(&bmp, imgname);
    imgSize = bmp.imgWidth * bmp.imgHeight * 3;
    matrix = createGaussianKernel(size, sigma);
    unsigned char* newData;
    newData = malloc(imgSize);
    FILE* f;
    char* kernelSource;
    size_t kernelSrcSize;
    // 读取kernel核代码
    if ((f = fopen("kernel.cl", "r")) == NULL) {
        fprintf(stderr, "Failed to load OpenCL kernel code.\n");
        return false;
    }
    kernelSource = malloc(MAX_SOURCE_SIZE);
    kernelSrcSize = fread(kernelSource, 1, MAX_SOURCE_SIZE, f);
    fclose(f);

    cl_platform_id platformID;
    cl_uint platformsN;
    cl_device_id deviceID;
    cl_uint devicesN;
    // 获得平台
    if (clGetPlatformIDs(1, &platformID, &platformsN) != CL_SUCCESS) {
        printf("Could not get the OpenCL Platform IDs\n");
        return false;
    }
    // 获得设备
    if (clGetDeviceIDs(platformID, CL_DEVICE_TYPE_DEFAULT, 1, &deviceID,
                       &devicesN) != CL_SUCCESS) {
        printf("Could not get the system's OpenCL device\n");
        return false;
    }
    // 创建上下文
    cl_context context = clCreateContext(NULL, 1, &deviceID, NULL, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Could not create a valid OpenCL context\n");
        return false;
    }
    // 创建命令队列
    cl_command_queue cmdQueue =
        clCreateCommandQueue(context, deviceID, 0, &ret);
    if (ret != CL_SUCCESS) {
        printf("Could not create an OpenCL Command Queue\n");
        return false;
    }
    // 创建数据数组对象
    cl_mem gpuImg =
        clCreateBuffer(context, CL_MEM_READ_ONLY, imgSize, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Unable to create the GPU image buffer object\n");
        return false;
    }
    cl_mem gpuGaussian = clCreateBuffer(
        context, CL_MEM_READ_ONLY, size * size * sizeof(float), NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Unable to create the GPU image buffer object\n");
        return false;
    }
    cl_mem gpuNewImg =
        clCreateBuffer(context, CL_MEM_WRITE_ONLY, imgSize, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Unable to create the GPU image buffer object\n");
        return false;
    }
    // 数据填充到数组之中
    if (clEnqueueWriteBuffer(cmdQueue, gpuImg, CL_TRUE, 0, imgSize, bmp.imgData,
                             0, NULL, NULL) != CL_SUCCESS) {
        printf("Error during sending the image data to the OpenCL buffer\n");
        return false;
    }
    if (clEnqueueWriteBuffer(cmdQueue, gpuGaussian, CL_TRUE, 0,
                             size * size * sizeof(float), matrix, 0, NULL,
                             NULL) != CL_SUCCESS) {
        printf(
            "Error during sending the gaussian kernel to the OpenCL buffer\n");
        return false;
    }
    // 使用源码创建程序
    cl_program program =
        clCreateProgramWithSource(context, 1, (const char**)&kernelSource,
                                  (const size_t*)&kernelSrcSize, &ret);
    free(kernelSource);
    if (ret != CL_SUCCESS) {
        printf("Error in creating an OpenCL program object\n");
        return false;
    }
    // 为设备构建(编译)程序
    if ((ret = clBuildProgram(program, 1, &deviceID, NULL, NULL, NULL)) !=
        CL_SUCCESS) {
        printf("Failed to build the OpenCL program\n");
        char* buildLog;
        buildLog = malloc(MAX_LOG_SIZE);
        if (clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG,
                                  MAX_LOG_SIZE, buildLog, NULL) != CL_SUCCESS) {
            printf("Could not get any Build info from OpenCL\n");
            free(buildLog);
            return false;
        }
        printf("**BUILD LOG**\n%s", buildLog);
        free(buildLog);
        return false;
    }
    // 创建卷积内核
    cl_kernel kernel = clCreateKernel(program, "gaussian_blur", &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create the OpenCL Kernel from the built program\n");
        return false;
    }
    // 设置内核参数
    if (clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&gpuImg) !=
        CL_SUCCESS) {
        printf("Could not set the kernel's \"gpuImg\" argument\n");
        return false;
    }
    if (clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&gpuGaussian) !=
        CL_SUCCESS) {
        printf("Could not set the kernel's \"gpuGaussian\" argument\n");
        return false;
    }
    if (clSetKernelArg(kernel, 2, sizeof(int), (void*)&bmp.imgWidth) !=
        CL_SUCCESS) {
        printf("Could not set the kernel's \"imageWidth\" argument\n");
        return false;
    }
    if (clSetKernelArg(kernel, 3, sizeof(int), (void*)&bmp.imgHeight) !=
        CL_SUCCESS) {
        printf("Could not set the kernel's \"imgHeight\" argument\n");
        return false;
    }
    if (clSetKernelArg(kernel, 4, sizeof(int), (void*)&size) != CL_SUCCESS) {
        printf("Could not set the kernel's \"gaussian size\" argument\n");
        return false;
    }
    if (clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&gpuNewImg) !=
        CL_SUCCESS) {
        printf("Could not set the kernel's \"gpuNewImg\" argument\n");
        return false;
    }
    // 定义工作项的空间维度和空间大小
    size_t globalWorkItemSize = imgSize;
    size_t workGroupSize = 64;
    // 通过执行API执行内核
    ret = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, &globalWorkItemSize,
                                 &workGroupSize, 0, NULL, NULL);
    // 将输出数组拷贝到主机端内存中
    ret = clEnqueueReadBuffer(cmdQueue, gpuNewImg, CL_TRUE, 0, imgSize, newData,
                              0, NULL, NULL);
    // 释放资源
    free(matrix);
    clFlush(cmdQueue);
    clFinish(cmdQueue);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(gpuImg);
    clReleaseMemObject(gpuGaussian);
    clReleaseMemObject(gpuNewImg);
    clReleaseCommandQueue(cmdQueue);
    clReleaseContext(context);
    bmp.imgData = newData;
    // 保存图片
    ImageBMP_Save(&bmp, "gpu_blur.bmp");
    return true;
}
