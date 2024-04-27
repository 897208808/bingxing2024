#include <immintrin.h>
#include <iostream>
#include<Windows.h>
#include<cmath>

using namespace std;

alignas(32) int cityx[1024] = {};
alignas(32) int cityy[1024] = {};
alignas(32) int dis[1024][1024] = {};

void jisuan(int num)
{
    for (int j = 0; j < num; j++)
    {
        __m256i x1 = _mm256_set1_epi32(cityx[j]);
        __m256i y1 = _mm256_set1_epi32(cityy[j]);
        for (int i = 0; i < num; i += 8)
        {
            __m256i x2 = _mm256_load_si256((__m256i*) & cityx[i]);
            __m256i y2 = _mm256_load_si256((__m256i*) & cityy[i]);

            __m256i xdiff = _mm256_sub_epi32(x2, x1);
            __m256i ydiff = _mm256_sub_epi32(y2, y1);
            __m256i ds_vec = _mm256_mullo_epi32(xdiff, xdiff);
            ds_vec = _mm256_add_epi32(ds_vec, _mm256_mullo_epi32(ydiff, ydiff));
            __m256i ds = _mm256_cvtps_epi32(_mm256_sqrt_ps(_mm256_cvtepi32_ps(ds_vec)));
            _mm256_store_si256((__m256i*) & (dis[j + 1][i + 1]), ds);
        }
    }
}

void chuan(int num)
{
    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < num; j++)
        {
            int dx = cityx[i] - cityx[j];
            int dy = cityy[i] - cityy[j];
            dis[i][j] = ceil(sqrt(dx * dx + dy * dy));
        }
    }
}


int main()
{
    int num[10] = { 100,200,300,400,500,600,700,800,900,1000 };
    for (int i = 0; i < 1024; i++)
    {
        cityx[i] = i;
        cityy[i] = 1024 - i;
    }
    long long time0, time1, freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    for (int i = 0; i < 10; i++)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&time0);
        for (int loop = 0; loop < 1000; loop++)
            jisuan(num[i]);
        QueryPerformanceCounter((LARGE_INTEGER*)&time1);
        cout << num[i] << " " << (time1 - time0) * 1000.0 / freq << "ms" << endl;
    }

    cout << endl << endl;
    for (int i = 0; i < 10; i++)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&time0);
        for (int loop = 0; loop < 1000; loop++)
            chuan(num[i]);
        QueryPerformanceCounter((LARGE_INTEGER*)&time1);
        cout << num[i] << " " << (time1 - time0) * 1000.0 / freq << "ms" << endl;
    }


    return 0;
}
