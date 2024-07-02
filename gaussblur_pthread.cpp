#include <windows.h>
#include <cmath>
#include <iostream>
using namespace std;

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define PI acos(-1)
#define NUM_THREADS 6

pthread_t thread[NUM_THREADS];

typedef struct {
    int threadId;
    int radius;
} threadParm_t;

threadParm_t parm[NUM_THREADS];

cv::Mat image;
double ***bitmap, ***resImage, ***tempImage;
double* filter;

void initArray(double**** array) {
    *array = new double**[image.rows];
    for (int i = 0; i < image.rows; i++) {
        (*array)[i] = new double*[image.cols];
        for (int j = 0; j < image.cols; j++) {
            (*array)[i][j] = new double[3];
        }
    }
}

void deleteArray(double*** array) {
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            delete[] array[i][j];
        }
        delete[] array[i];
    }
    delete[] array;
}

void readImage() {
    for (size_t row = 0; row < image.rows; row++) {
        unsigned char* row_ptr = image.ptr<unsigned char>(row);
        for (size_t col = 0; col < image.cols; col++) {
            unsigned char* data = &row_ptr[col * image.channels()];
            for (int k = 0; k < image.channels(); k++) {
                bitmap[row][col][k] = (double)(int)data[k] / 255;
            }
        }
    }
}

double gaussFunc1D(int radius, int i) {
    double sigma = (double)radius / 3;
    return exp(-(double)(i * i) / 2 / sigma / sigma) / sigma / sqrt(2 * PI);
}

void gaussFilter1D(int radius) {
    int length = 2 * radius + 1;
    filter = new double[length];
    double sum = 0;
    for (int i = 0; i < length; i++) {
        filter[i] = gaussFunc1D(radius, i - radius);
        sum += filter[i];
    }
    for (int i = 0; i < length; i++) {
        filter[i] /= sum;
    }
}

int edge(int x, int radius, int cols) {
    int x1 = radius + x;
    if (x1 < 0 || x1 >= cols) {
        return x - radius;
    }
    return x1;
}

void* gaussBlur1D_row(void* arg) {
    threadParm_t* parm = (threadParm_t*)arg;
    int radius = parm->radius;
    int id = parm->threadId;
    int cols = image.cols, rows = image.rows;
    int delta = rows / NUM_THREADS;
    int begin = delta * id, end = delta * (id + 1);
    if (id == NUM_THREADS - 1) {
        end = rows;
    }
    for (int i = begin; i < end; i++) {
        for (int j = 0; j < cols; j++) {
            for (int k = 0; k < 3; k++) {
                tempImage[i][j][k] = 0;
                for (int d = -radius; d <= radius; d++) {
                    int c = edge(j, d, cols);
                    tempImage[i][j][k] += filter[d + radius] * bitmap[i][c][k];
                }
                tempImage[i][j][k] =
                    tempImage[i][j][k] > 1 ? 1 : tempImage[i][j][k];
            }
        }
    }
    return NULL;
}

void* gaussBlur1D_col(void* arg) {
    threadParm_t* parm = (threadParm_t*)arg;
    int radius = parm->radius;
    int id = parm->threadId;
    int cols = image.cols, rows = image.rows;
    int delta = cols / NUM_THREADS;
    int begin = delta * id, end = delta * (id + 1);
    for (int j = begin; j < end; j++) {
        for (int i = 0; i < rows; i++) {
            for (int k = 0; k < 3; k++) {
                resImage[i][j][k] = 0;
                for (int d = -radius; d <= radius; d++) {
                    int r = edge(i, d, rows);
                    resImage[i][j][k] +=
                        filter[d + radius] * tempImage[r][j][k];
                }
                resImage[i][j][k] =
                    resImage[i][j][k] > 1 ? 1 : tempImage[i][j][k];
            }
        }
    }
    return NULL;
}

void writeImage(cv::Mat image, double*** bitmap) {
    for (size_t row = 0; row < image.rows; row++) {
        unsigned char* row_ptr = image.ptr<unsigned char>(row);
        for (size_t col = 0; col < image.cols; col++) {
            unsigned char* data = &row_ptr[col * image.channels()];
            for (int k = 0; k < image.channels(); k++) {
                data[k] = (unsigned char)(int)(bitmap[row][col][k] * 255);
            }
        }
    }
}

int main() {
    image = cv::imread("D:/1.jpg");
    initArray(&bitmap);
    initArray(&resImage);
    initArray(&tempImage);
    readImage();
    long freq, head, tail;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);
    gaussFilter1D(10);
    for (int i = 0; i < NUM_THREADS; i++) {
        parm[i] = threadParm_t{i, 10};
        pthread_create(&thread[i], NULL, gaussBlur1D_row, (void*)&parm[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        parm[i] = threadParm_t{i, 10};
        pthread_create(&thread[i], NULL, gaussBlur1D_col, (void*)&parm[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    cout << "1d: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    cv::Mat newImage = image.clone();
    writeImage(newImage, resImage);
    cv::imwrite("D:/1_2.jpg", newImage);
    cout << "finish" << endl;
    deleteArray(bitmap);
    deleteArray(resImage);
    deleteArray(tempImage);
    return 0;
}
