#include<iostream>
#include<Windows.h>

using namespace std;

const int N = 100000;
int a[N+2] = {};
int out1[2] = {};
int sum1 = 0;
int sum2 = 0;
int sum3 = 0;
int sum4 = 0;

void init()
{
	for (int i = 0; i < N; i++)
	{
		a[i] = i;
	}
	return;
}

//平凡算法
void solve1()
{
	for (int i = 0; i < N; i++)
		sum1 += a[i];
	return;
}

//两路链式相加
void solve2()
{
	for (int i = 0; i < N; i += 2)
	{
		out1[0] += a[i];
		out1[1] += a[i + 1];
	}
	sum2 = out1[0] + out1[1];
	return;
}

//2*1循环展开
void solve3()
{
	for (int i = 0; i < N; i += 2)
	{
		sum3 += a[i];
		sum3 += a[i + 1];
	}
	return;
}

//3*1循环展开
void solve4()
{
	for (int i = 0; i < N; i += 3)
	{
		sum4 += a[i];
		sum4 += a[i + 1];
		sum4 += a[i + 2];
	}
	return;
}

//void solve3()
//{
//	for (int m = N; m > 1; m /= 2)
//	{
//		for (int i = 0; i < m / 2 + 1; i++)
//			a2[i] = a2[i * 2] + a2[i * 2 + 1];
//	}
//	sum3 = a2[0];
//}

//void solve3(int n)
//{
//	if (n == 1)
//	{
//		sum3 = a[0];
//		return;
//	}
//	else
//	{
//		for (int i = 0; i < n / 2 + 1; i++)
//			a[i] += a[n/2 + i];
//		n = n/2 ;
//		solve3(n);
//	}
//}

int main()
{
	init();
	long long time0, time1, freq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&time0);
	for(int i = 0; i < N; i++)
		solve1();
	QueryPerformanceCounter((LARGE_INTEGER*)&time1);
	cout << "平凡算法的时间为" << (time1 - time0) * 1000.0 / freq << "ms" << endl;

	QueryPerformanceCounter((LARGE_INTEGER*)&time0);
	for (int i = 0; i < N; i++)
		solve2();
	QueryPerformanceCounter((LARGE_INTEGER*)&time1);
	cout << "两路链式算法的时间为" << (time1 - time0) * 1000.0 / freq << "ms" << endl;

	QueryPerformanceCounter((LARGE_INTEGER*)&time0);
	for (int i = 0; i < N; i++)
		solve3();
	QueryPerformanceCounter((LARGE_INTEGER*)&time1);
	cout << "2*1循环展开算法的时间为" << (time1 - time0) * 1000.0 / freq << "ms" << endl;

	QueryPerformanceCounter((LARGE_INTEGER*)&time0);
	for (int i = 0; i < N; i++)
		solve4();
	QueryPerformanceCounter((LARGE_INTEGER*)&time1);
	cout << "3*1循环展开算法的时间为" << (time1 - time0) * 1000.0 / freq << "ms" << endl;
	cout << sum1 << "," << sum2 << "," << sum3 << "," << sum4 << endl;

	return 0;
}
