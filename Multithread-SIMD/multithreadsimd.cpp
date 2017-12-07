// MultiThread-SIMD.cpp : Defines the entry point for the console application.
//
#include <Windows.h>
#include <intrin.h>
#include "immintrin.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <thread>

using namespace std;
// definicion de directivas

unsigned int getNumbersProcessor() {
	SYSTEM_INFO siSysInfo;

	// Copy the hardware information to the SYSTEM_INFO structure. 
	GetSystemInfo(&siSysInfo);

	return siSysInfo.dwNumberOfProcessors;
}

#define NTHREADS					getNumbersProcessor()
#define TIMES						10
#define NTIMES						200							// Number of repetitions to get suitable times
#define SIZE						(1024*1024)					// Number of elements in the array
#define NUMBER_FLOAT				sizeof(__m256) / sizeof(float)
#define PRINT_FUNCTIONS				false
#define PRINT_TIMER_FUNCTION		false
#define PRINT_TIMER					true

//Threads
HANDLE* hThreadArray = new HANDLE[NTHREADS];
int* sizes;

// Timer
LARGE_INTEGER frequency;
LARGE_INTEGER tStart;
LARGE_INTEGER tEnd;
double dElapsedTimeS;

//definicion de los atributos a usar en las funciones
float* u;			//vector usado en Dif2
float* t;			//vector usado en Sub
float* w;			//vector usado en ContarPositivos
float* v;			//vector resultante de k * Dif2()

					//atributos de return
float* r;			// vector resultante de op1
unsigned int k;		//numero de positivos op2
unsigned int* arrayK;	//almacena el valor devuelto de positivos por cada hilo
float* s;			// vector resultante de op3

					//devuelve un vector de tamaño SIZE
float* createVector() {
	float* vector = (float *)_aligned_malloc(SIZE * sizeof(float), sizeof(__m256i));

	for (int i = 0; i < SIZE; i++) {
		float r = -1 + 2 * float((double)rand() / (double)(RAND_MAX)); // Aqui se explica por que hacerlo con double: https://stackoverflow.com/questions/13408990/how-to-generate-random-float-number-in-c#comment56659626_13409133
		vector[i] = r;
	}
	return vector;
}

// elimina vector
void removeVector(float* vector) {
	_aligned_free(vector);
}

//espera por todos los threads creados
void wait() {
	WaitForMultipleObjects(NTHREADS, hThreadArray, true, INFINITE);
}

//genera la lista de indices para cada hilo
void generateSizes() {
	sizes = (int *)malloc(sizeof(int) * NTHREADS);
	for (int i = 0; i < NTHREADS; i++) {
		sizes[i] = i * (SIZE / NTHREADS);
	}
}

////metodos usados para los procedimientos en MultiThread
DWORD WINAPI Dif2Proc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	unsigned int tamaño = (SIZE / NTHREADS);
	__m256 number2 = _mm256_set_ps(2, 2, 2, 2, 2, 2, 2, 2);
	for (int i = indexInt; i < (indexInt + tamaño - 1) / NUMBER_FLOAT; i++) {
			__m256 valuei = *(__m256 *)&u[(i + 1) * NUMBER_FLOAT];
			__m256 valuei_minus_1 = *(__m256 *)&u[i * NUMBER_FLOAT];
			__m256 value = _mm256_sub_ps(valuei, valuei_minus_1);
			value = _mm256_div_ps(value, number2);

			float *p = (float*)&value;							// Pointer p points to the first 32 integer in the packet
			for (int j = 0; j < NUMBER_FLOAT; j++) {
				r[i*NUMBER_FLOAT + j] = *p;
				p++;
				if (PRINT_FUNCTIONS)
					printf("La diferencia entre dos valores es %f\n", r[i + j]);
		}
	}
	return 0;
}

DWORD WINAPI CountPositiveValuesProc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	unsigned int tamaño = (SIZE / NTHREADS);
	__m256 mask = _mm256_set_ps(0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000);
	for (int j = indexInt; j < (indexInt + tamaño) / NUMBER_FLOAT; j++) {
		__m256 value = *(__m256*)&w[j * NUMBER_FLOAT];
		__m256 and = _mm256_and_ps(value, mask);	// mascara para mirar el bit mas significativo

		float *p = (float*)&and;
		for (int i = 0; i < NUMBER_FLOAT; i++) {
			if (*(p + i) != 0x80000000) {
				arrayK[indexInt / tamaño]++;
			}
		}
	}
	return 0;
}

DWORD WINAPI SubProc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	unsigned int tamaño = (SIZE / NTHREADS);
		__m256 kIntrisincs = _mm256_set_ps((float)k, (float)k, (float)k, (float)k, (float)k, (float)k, (float)k, (float)k);
	for (int i = indexInt; i < (indexInt + tamaño - 1) / NUMBER_FLOAT; i++) {
		__m256 valueR = *(__m256*)&r[i * NUMBER_FLOAT];
		__m256 valueT = *(__m256*)&t[i*NUMBER_FLOAT];
		__m256 sub = _mm256_sub_ps(_mm256_mul_ps(kIntrisincs, valueR), valueT);	//v[i] - t[i]

		float* p = (float*)&sub;
		for (int j = 0; j < NUMBER_FLOAT; j++) {
			s[i * NUMBER_FLOAT + j] = *(p + j);

			if (PRINT_FUNCTIONS)
				printf("La resta es %f\n", s[i + j]);
		}
	}
	return 0;
}


//funciones usadas para crear los hilos necesarios
void Dif2() {
	for (unsigned int i = 0; i < NTHREADS; i++) {
		hThreadArray[i] = CreateThread(NULL, 0, Dif2Proc, &sizes[i], 0, NULL);
	}
	wait();
}

void CountPositiveValues() {
	for (unsigned int i = 0; i < NTHREADS; i++) {
		arrayK[i] = 0;
		hThreadArray[i] = CreateThread(NULL, 0, CountPositiveValuesProc, &sizes[i], 0, NULL);
	}

	wait();
	k = 0;

	for (unsigned int i = 0; i < NTHREADS; i++) {
		k += arrayK[i];
	}

	if (PRINT_FUNCTIONS)
		printf("El contador de numeros positivos es %d\n", k);
}

void Sub() {
	//codigo del programa
	for (unsigned int i = 0; i < NTHREADS; i++) {
		hThreadArray[i] = CreateThread(NULL, 0, SubProc, &sizes[i], 0, NULL);
	}
	wait();
}

// funcion usada para calcular el tiempo de la funcion pasada como parametro
double timer(void(*function)(void)) {
	// Get clock frequency in Hz
	QueryPerformanceFrequency(&frequency);
	// Get initial clock count
	QueryPerformanceCounter(&tStart);
	// Code to be measured
	function();
	// Get final clock count
	QueryPerformanceCounter(&tEnd);
	// Compute the elapsed time in seconds
	dElapsedTimeS = (tEnd.QuadPart - tStart.QuadPart) / (double)frequency.QuadPart;
	// Print the elapsed time
	if (PRINT_TIMER_FUNCTION)
		printf("Elapsed time in seconds: %f\n", dElapsedTimeS);
	// Return the elapsed time if it'll util
	return dElapsedTimeS;
}

// funcion main del programa singleThread
int main() {
	time_t ti;
	srand((unsigned)time(&ti)); // Informacion sobre srand https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm

	double times[TIMES];
	generateSizes();		//generar la lista de indices

	for (int j = 0; j < TIMES; j++) {
		times[j] = 0;			//aseguramos valores reales
		for (int i = 0; i < NTIMES; i++) {
			//vectores aleatorios
			u = createVector();
			w = createVector();
			t = createVector();

			//vectores aux
			arrayK = (unsigned int *)malloc(sizeof(unsigned int) * (NTHREADS));

			//vectores resultantes
			r = (float *)malloc(sizeof(float) * (SIZE - 1));
			s = (float *)malloc(sizeof(float) * (SIZE - 1));

			times[j] += timer(Dif2);
			times[j] += timer(CountPositiveValues);
			times[j] += timer(Sub);

			std::free(arrayK);
			removeVector(u);
			removeVector(w);
			removeVector(t);
			/*removeVector(s);
			removeVector(r);*/
		}

		if (PRINT_TIMER)
			printf("Elapsed total time in seconds: %f\n", times[j]);
	}
}
