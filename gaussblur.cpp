#include <windows.h>
#include <cmath>
#include <iostream>
using namespace std;

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define PI acos(-1)

double*** bitmap;

void initBitmap(cv::Mat image) {
    bitmap = new double**[image.rows];
    for (int i = 0; i < image.rows; i++) {
        bitmap[i] = new double*[image.cols];
        for (int j = 0; j < image.cols; j++) {
            bitmap[i][j] = new double[3];
        }
    }
}

void deleteBitmap(cv::Mat image) {
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            delete[] bitmap[i][j];
        }
        delete[] bitmap[i];
    }
    delete[] bitmap;
}

void readImage(cv::Mat image) {
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

double gaussFunc2D(int radius, int i, int j) {
    double sigma = (double)radius / 3;
    return exp(-(double)(i * i + j * j) / 2 / sigma / sigma) /
           (2 * PI * sigma * sigma);
}

double gaussFunc1D(int radius, int i) {
    double sigma = (double)radius / 3;
    return exp(-(double)(i * i) / 2 / sigma / sigma) / sigma / sqrt(2 * PI);
}

double** gaussFilter2D(int radius) {
    int length = 2 * radius + 1;
    double** filter = new double*[length];
    for (int i = 0; i < length; i++) {
        filter[i] = new double[length];
    }
    double sum = 0;
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < length; j++) {
            filter[i][j] = gaussFunc2D(radius, i - radius, j - radius);
            sum += filter[i][j];
        }
    }
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < length; j++) {
            filter[i][j] /= sum;
        }
    }
    return filter;
}

double* gaussFilter1D(int radius) {
    int length = 2 * radius + 1;
    double* filter = new double[length];
    double sum = 0;
    for (int i = 0; i < length; i++) {
        filter[i] = gaussFunc1D(radius, i - radius);
        sum += filter[i];
    }
    for (int i = 0; i < length; i++) {
        filter[i] /= sum;
    }
    return filter;
}

int edge(int x, int radius, int cols) {
    int x1 = radius + x;
    if (x1 < 0 || x1 >= cols) {
        return x - radius;
    }
    return x1;
}

double*** gaussBlur2D(cv::Mat image, int radius) {
    int cols = image.cols, rows = image.rows;
    double*** newImage = new double**[rows];
    double** filter = gaussFilter2D(radius);
    for (int i = 0; i < rows; i++) {
        newImage[i] = new double*[cols];
        for (int j = 0; j < cols; j++) {
            newImage[i][j] = new double[3];
            for (int k = 0; k < 3; k++) {
                for (int row = -radius; row <= radius; row++) {
                    int r = edge(i, row, rows);
                    for (int col = -radius; col <= radius; col++) {
                        int c = edge(j, col, cols);
                        newImage[i][j][k] +=
                            filter[radius + row][radius + col] *
                            bitmap[r][c][k];
                    }
                }
                newImage[i][j][k] =
                    newImage[i][j][k] > 1 ? 1 : newImage[i][j][k];
            }
        }
    }
    for (int i = 0; i < radius; i++) {
        delete[] filter[i];
    }
    delete[] filter;
    return newImage;
}

double*** gaussBlur1D(cv::Mat image, int radius) {
    int cols = image.cols, rows = image.rows;
    double*** newImage = new double**[rows];
    double* filter = gaussFilter1D(radius);
    for (int i = 0; i < rows; i++) {
        newImage[i] = new double*[cols];
        for (int j = 0; j < cols; j++) {
            newImage[i][j] = new double[3];
            for (int k = 0; k < 3; k++) {
                newImage[i][j][k] = 0;
                for (int d = -radius; d <= radius; d++) {
                    int c = edge(j, d, cols);
                    newImage[i][j][k] += filter[d + radius] * bitmap[i][c][k];
                }
                newImage[i][j][k] =
                    newImage[i][j][k] > 1 ? 1 : newImage[i][j][k];
            }
        }
    }
    double*** resImage = new double**[rows];
    for (int i = 0; i < rows; i++) {
        resImage[i] = new double*[cols];
        for (int j = 0; j < cols; j++) {
            resImage[i][j] = new double[3];
            for (int k = 0; k < 3; k++) {
                resImage[i][j][k] = 0;
                for (int d = -radius; d <= radius; d++) {
                    int r = edge(i, d, rows);
                    resImage[i][j][k] += filter[d + radius] * newImage[r][j][k];
                }
                resImage[i][j][k] =
                    resImage[i][j][k] > 1 ? 1 : newImage[i][j][k];
            }
        }
    }
    delete[] filter;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            delete[] newImage[i][j];
        }
        delete[] newImage[i];
    }
    delete[] newImage;
    return resImage;
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
    cv::Mat image;
    image = cv::imread("D:/1.jpg");
    if (image.data == nullptr) {
        cerr << "wrong image\n";
        return 0;
    }
    initBitmap(image);
    readImage(image);
    long freq, head, tail;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);
    double*** resImage = gaussBlur1D(image, 10);
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    cout << "1d: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    cv::Mat newImage = image.clone();
    writeImage(newImage, resImage);
    cv::imwrite("D:/1_1.jpg", newImage);
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            delete[] resImage[i][j];
        }
        delete[] resImage[i];
    }
    delete[] resImage;
    return 0;
}
