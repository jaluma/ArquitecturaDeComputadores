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

using namespace std;
// definicion de directivas

unsigned int getNumbersProcessor() {
	SYSTEM_INFO siSysInfo;

	// Copy the hardware information to the SYSTEM_INFO structure. 
	GetSystemInfo(&siSysInfo);

	return siSysInfo.dwNumberOfProcessors;
}

#define NTHREADS					getNumbersProcessor()
#define THREADS_USABLES				NTHREADS - threadsUsed
#define TIMES						10
#define NTIMES						200							// Number of repetitions to get suitable times
#define SIZE						(1024*1024)					// Number of elements in the array
#define PRINT_FUNCTIONS				false
#define PRINT_TIMER_FUNCTION		false
#define PRINT_TIMER					true

struct CalculateTime
{
	void(*function)(void) = NULL;
	double timer = 0;

};

//Threads
unsigned int threadsUsed = 0;
struct CalculateTime ct;
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
float* v;			//vector resultante de k * Dif2()

					//atributos de return
float* r;			// vector resultante de op1
unsigned int k;		//numero de positivos op2
float* s;			// vector resultante de op3

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

void wait() {
	WaitForMultipleObjects(threadsUsed, hThreadArray, true, 0);
	threadsUsed = 0;
}

void generateThread(DWORD WINAPI function(LPVOID a), LPVOID param) {
	if (threadsUsed >= NTHREADS) {
		wait();
	}
	else {
		hThreadArray[threadsUsed] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, param, 0, NULL);
		threadsUsed++;
	}
}

//metodos usados para los procedimientos en MultiThread
DWORD WINAPI Dif2Proc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	for (int i = indexInt; i < indexInt + (SIZE / THREADS_USABLES) - 1; i++) {
		r[i] = (u[i + 1] - u[i]) / 2.0;

		if (PRINT_FUNCTIONS)
			printf("La diferencia entre dos valores es %f\n", r[i]);
	}
	return 0;
}

DWORD WINAPI CountPositiveValuesProc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	for (int i = indexInt; i < indexInt + (SIZE / THREADS_USABLES); i++) {
		if (w[i] >= 0.0)
			k++;

		if (PRINT_FUNCTIONS)
			printf("El contador de numeros positivos es %d\n", k);
	}
	return 0;
}

DWORD WINAPI MultProc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	for (int i = indexInt; i < indexInt + (SIZE / THREADS_USABLES) - 1; i++) {
		v[i] = k * r[i];
	}
	return 0;
}

DWORD WINAPI SubProc(LPVOID index) {			//int index
	int indexInt = *reinterpret_cast<int*>(index);
	for (int i = indexInt; i < indexInt + (SIZE / THREADS_USABLES); i++) {
		s[i] = v[i] - t[i];

		if (PRINT_FUNCTIONS)
			printf("La resta es %f\n", s[i]);
	}
	return 0;
}

void Dif2() {

	unsigned int tamaño = SIZE / THREADS_USABLES;

	for (unsigned int i = 0; i < THREADS_USABLES; i++) {
		int position = i*tamaño;
		generateThread(Dif2Proc, &position);
	}

}

void CountPositiveValues() {
	k = 0;

	unsigned int tamaño = SIZE / THREADS_USABLES;

	for (unsigned int i = 0; i < THREADS_USABLES; i++) {
		int position = i*tamaño;
		generateThread(CountPositiveValuesProc, &position);
	}
}

void Sub() {
	//inicalizacion del vector V
	unsigned int tamaño = SIZE / THREADS_USABLES;

	for (unsigned int i = 0; i < THREADS_USABLES; i++) {
		int position = i*tamaño;
		generateThread(MultProc, &position);
	}

	//codigo del programa
	tamaño = SIZE / THREADS_USABLES;

	for (int i = 0; i < THREADS_USABLES; i++) {
		int position = i*tamaño;
		generateThread(SubProc, &position);
	}
	//eliminar de  memoria el vector V
	removeVector(v);
}


// funcion usada para calcular el tiempo de la funcion pasada como parametro
DWORD WINAPI timer(LPVOID arg) {
	// Get clock frequency in Hz
	QueryPerformanceFrequency(&frequency);
	// Get initial clock count
	QueryPerformanceCounter(&tStart);
	// Code to be measured
	ct.function();
	// Get final clock count
	QueryPerformanceCounter(&tEnd);
	// Compute the elapsed time in seconds
	dElapsedTimeS = (tEnd.QuadPart - tStart.QuadPart) / (double)frequency.QuadPart;
	// Print the elapsed time
	if (PRINT_TIMER_FUNCTION)
		printf("Elapsed time in seconds: %f\n", dElapsedTimeS);
	// Return the elapsed time if it'll util
	ct.timer += dElapsedTimeS;

	return 0;
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
			r = (float *)malloc(sizeof(float) * (SIZE - 1));
			v = (float *)malloc(sizeof(float) * (SIZE - 1));
			s = (float *)malloc(sizeof(float) * (SIZE - 1));

			ct.function = Dif2;
			timer(NULL);
			ct.function = CountPositiveValues;
			timer(NULL);
			wait();
			ct.function = Sub;
			timer(NULL);
			times[j] = ct.timer;
			wait();
			////usado para crear el hilo
			//ct.timer = times[j];

			////ct.function = Dif2;
			//generateThread(timer, Dif2);

			////ct.function = countPositiveValues;
			//generateThread(timer, countPositiveValues);

			//wait();

			////ct.function = Sub;
			//generateThread(timer, Sub);

			//wait();

			removeVector(u);
			removeVector(w);
			removeVector(t);
			removeVector(s);
			removeVector(r);
		}

		if (PRINT_TIMER)
			printf("Elapsed total time in seconds: %f\n", times[j]);
	}
}

// genera un archivo *.csv con los tiempos que devuelve la funcion timer
//const string currentDateTime() {
//	time_t now = time(0);
//	struct tm  tstruct;
//	char buf[80];
//	localtime_s(&tstruct, &now);
//	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
//
//	return buf;
//}
//
//void generateFile(double* times) {
//	string nameFile = currentDateTime() + ".csv";
//	ofstream archivo(nameFile);
//
//	for (int i = 0; i < TIMES; i++) {
//		archivo << times[i];
//		archivo << ";";
//	}
//	archivo.close();
//}