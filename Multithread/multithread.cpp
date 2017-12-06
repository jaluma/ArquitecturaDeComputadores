// MultiThread.cpp : Defines the entry point for the console application.

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
#include <atomic>
#include <array> 

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
#define PRINT_FUNCTIONS				false
#define PRINT_TIMER_FUNCTION		false
#define PRINT_TIMER					true

//Threads
HANDLE* hThreadArray = new HANDLE[NTHREADS];

// Timer
LARGE_INTEGER frequency;
LARGE_INTEGER tStart;
LARGE_INTEGER tEnd;
double dElapsedTimeS;

//definicion de los atributos a usar en las funciones
float* u;			//vector usado en Dif2
float* t;			//vector usado en Sub
float* w;			//vector usado en ContarPositivos
std::array<std::atomic<float>, (SIZE - 1)> v;			//vector resultante de k * Dif2()

//atributos de return
std::array<std::atomic<float>, (SIZE - 1)> r;				// vector resultante de op1
//float* r;
//unsigned int k;		//numero de positivos op2
std::atomic<int> k;
std::array<std::atomic<float>, (SIZE - 1)> s;				// vector resultante de op3
//float* s;

					//devuelve un vector de tamaño SIZE
float* createVector() {
	float* vector = (float *)malloc(sizeof(float) * SIZE);

	for (int i = 0; i < SIZE; i++) {
		float r = -1 + 2 * float((double)rand() / (double)(RAND_MAX)); // Aqui se explica por que hacerlo con double: https://stackoverflow.com/questions/13408990/how-to-generate-random-float-number-in-c#comment56659626_13409133
		vector[i] = r;
	}
	return vector;
}

// elimina vector
void removeVector(float* vector) {
	std::free(vector);
}

//esperar por todos los threads creados
void wait() {
	for (unsigned int i = 0; i < NTHREADS; i++)
		WaitForSingleObject(hThreadArray[i], INFINITE);
}

////metodos usados para los procedimientos en MultiThread
static DWORD WINAPI Dif2Proc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	unsigned int tamaño = (SIZE / NTHREADS);
	for (unsigned int i = indexInt; i < indexInt + tamaño - 1; i++) {
		//std::atomic_init((u[i + 1] - u[i]) / 2.0, i);
		r[i] = (u[i + 1] - u[i]) / 2.0;

		/*if (PRINT_FUNCTIONS)
			printf("La diferencia entre dos valores es %f\n", r[i]);*/
	}
	return 0;
}

static DWORD WINAPI CountPositiveValuesProc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	unsigned int tamaño = (SIZE / NTHREADS);
	for (unsigned int i = indexInt; i < indexInt + tamaño; i++) {
		if (w[i] >= 0.0)
			k++;
	}
	return 0;
}

static DWORD WINAPI SubProc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	unsigned int tamaño = (SIZE / NTHREADS);
	for (unsigned int i = indexInt; i < indexInt + tamaño - 1; i++) {
		v[i] = k * r.at(i);
		s[i] = v.at(i) - t[i];

		/*if (PRINT_FUNCTIONS)
			printf("La resta es %f\n", s[i]);*/
	}
	return 0;
}

void Dif2() {

	unsigned int tamaño = SIZE / NTHREADS;

	for (unsigned int i = 0; i < NTHREADS; i++) {
		int position = i*tamaño;
		hThreadArray[i] = CreateThread(NULL, 0, Dif2Proc, &position, 0, NULL);
	}
	wait();
}

void CountPositiveValues() {
	k = 0;

	unsigned int tamaño = SIZE / NTHREADS;

	for (unsigned int i = 0; i < NTHREADS; i++) {
		int position = i*tamaño;
		hThreadArray[i] = CreateThread(NULL, 0, CountPositiveValuesProc, &position, 0, NULL);
	}

	if (PRINT_FUNCTIONS)
		printf("El contador de numeros positivos es %d\n", (int)k);
	wait();
}

void Sub() {
	//inicalizacion del vector V
	unsigned int tamaño = SIZE / NTHREADS;

	//codigo del programa
	for (unsigned int i = 0; i < NTHREADS; i++) {
		int position = i*tamaño;
		hThreadArray[i] = CreateThread(NULL, 0, SubProc, &position, 0, NULL);
	}
	//eliminar de  memoria el vector V
	//removeVector(v);
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

	for (int j = 0; j < TIMES; j++) {
		times[j] = 0;			//aseguramos valores reales
		for (int i = 0; i < NTIMES; i++) {
			//vectores aleatorios
			u = createVector();
			w = createVector();
			t = createVector();

			//vectores resultantes
			//r = (float *)malloc(sizeof(float) * (SIZE - 1));
			//v = (float *)malloc(sizeof(float) * (SIZE - 1));
			//s = (float *)malloc(sizeof(float) * (SIZE - 1));

			times[j] += timer(Dif2);
			wait();
			times[j] += timer(CountPositiveValues);
			wait();
			times[j] += timer(Sub);
			wait();

			removeVector(u);
			removeVector(w);
			removeVector(t);
			//removeVector(s);
			//removeVector(r);
		}

		if (PRINT_TIMER)
			printf("Elapsed total time in seconds: %f\n", times[j]);
	}
}