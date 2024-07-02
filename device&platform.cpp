#include <CL/cl.h>
#include <malloc.h>
#include <string.h>
#include <iostream>

using namespace std;

int main() {
    cl_platform_id* platform;
    cl_uint num_platform;
    cl_int err;
    // 获得platform数目
    err = clGetPlatformIDs(0, NULL, &num_platform);
    // 分配空间
    platform = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platform);
    err = clGetPlatformIDs(num_platform, platform, NULL);
    for (int i = 0; i < num_platform; i++) {
        size_t size;
        // 获取name
        err = clGetPlatformInfo(platform[i], CL_PLATFORM_NAME, 0, NULL, &size);
        char* name = (char*)malloc(size);
        err =
            clGetPlatformInfo(platform[i], CL_PLATFORM_NAME, size, name, NULL);
        cout << "CL_PLATFORM_NAME:" << name << endl;
        // 获取vendor
        err =
            clGetPlatformInfo(platform[i], CL_PLATFORM_VENDOR, 0, NULL, &size);
        char* vendor = (char*)malloc(size);
        err = clGetPlatformInfo(platform[i], CL_PLATFORM_VENDOR, size, vendor,
                                NULL);
        cout << "CL_PLATFORM_VENDOR:" << vendor << endl;
        // 获取version
        err =
            clGetPlatformInfo(platform[i], CL_PLATFORM_VERSION, 0, NULL, &size);
        char* version = (char*)malloc(size);
        err = clGetPlatformInfo(platform[i], CL_PLATFORM_VERSION, size, version,
                                NULL);
        cout << "CL_PLATFORM_VERSION:" << version << endl;
        // 获取profile
        err =
            clGetPlatformInfo(platform[i], CL_PLATFORM_PROFILE, 0, NULL, &size);
        char* profile = (char*)malloc(size);
        err = clGetPlatformInfo(platform[i], CL_PLATFORM_PROFILE, size, profile,
                                NULL);
        cout << "CL_PLATFORM_PROFILE:" << profile << endl;
        // 获取extensions
        err = clGetPlatformInfo(platform[i], CL_PLATFORM_EXTENSIONS, 0, NULL,
                                &size);
        char* extensions = (char*)malloc(size);
        err = clGetPlatformInfo(platform[i], CL_PLATFORM_EXTENSIONS, size,
                                extensions, NULL);
        cout << "CL_PLATFORM_EXTENSIONS:" << extensions << endl;

        cout << endl;
        // 释放空间
        free(name);
        free(vendor);
        free(version);
        free(profile);
        free(extensions);
    }
    // 选用GPU平台输出device信息
    cl_platform_id platform1 = platform[1];
    struct {
        cl_device_type type;
        const char* name;
        cl_uint count;
    } devices[] = {
        {CL_DEVICE_TYPE_CPU, "CL_DEVICE_TYPE_CPU", 0},
        {CL_DEVICE_TYPE_GPU, "CL_DEVICE_TYPE_GPU", 0},
        {CL_DEVICE_TYPE_ACCELERATOR, "CL_DEVICE_TYPE_ACCELERATOR", 0}};
    const int NUM_OF_DEVICE_TYPES = sizeof(devices) / sizeof(devices[0]);
    // 获取name
    for (int i = 0; i < NUM_OF_DEVICE_TYPES; ++i) {
        err =
            clGetDeviceIDs(platform1, devices[i].type, 0, 0, &devices[i].count);
        if (CL_DEVICE_NOT_FOUND == err) {
            devices[i].count = 0;
            err = CL_SUCCESS;
        }
        cout << " " << devices[i].name << ": " << devices[i].count << endl;
    }
    // 获取device相关信息
    for (int type_index = 0; type_index < NUM_OF_DEVICE_TYPES; ++type_index) {
        cl_uint cur_num_of_devices = devices[type_index].count;
        if (cur_num_of_devices == 0) {
            continue;
        }
        cl_device_id* devices_of_type = new cl_device_id[cur_num_of_devices];
        err = clGetDeviceIDs(platform1, devices[type_index].type,
                             cur_num_of_devices, devices_of_type, 0);

        for (cl_uint device_index = 0; device_index < cur_num_of_devices;
             ++device_index) {
            cout << endl
                 << devices[type_index].name << "[" << device_index << "]\n";
            cl_device_id device = devices_of_type[device_index];
        }
    }
    free(platform);
    return 0;
}
