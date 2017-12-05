// Single thread application

#include <Windows.h>
#include <intrin.h>
#include "immintrin.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

using namespace std;
// definicion de directivas

#define TIMES						10
#define NTIMES						200							// Number of repetitions to get suitable times
#define SIZE						(1024*1024)					// Number of elements in the array
#define PRINT_FUNCTIONS				false
#define PRINT_TIMER_FUNCTION		false
#define PRINT_TIMER					true

// Timer
LARGE_INTEGER frequency;
LARGE_INTEGER tStart;
LARGE_INTEGER tEnd;
double dElapsedTimeS;

//definicion de los atributos a usar en las funciones
float* u;			//vector usado en Dif2
float* t;			//vector usado en Sub
float* w;			//vector usado en ContarPositivos																																							                                                                       
//atributos de return
float* r;			// vector resultante de op1
unsigned int k;		//numero de positivos op2
float* s;			// vector resultante de op3


//devuelve un vector de tamaño SIZE
float* createVector() {
	float* vector = (float *)malloc(sizeof(float) * SIZE);

	for (int i = 0; i < SIZE; i++) {
		float r = -1 + 2 * float((double)rand() / (double)(RAND_MAX)); // Aquí se explica por qué hacerlo con double: https://stackoverflow.com/questions/13408990/how-to-generate-random-float-number-in-c#comment56659626_13409133
		vector[i] = r;
	}
	return vector;
}

// elimina vector
void removeVector(float* vector) {
	free(vector);
}

void Dif2() {
	r = (float *)malloc(sizeof(float) * (SIZE - 1));

	for (int i = 0; i < SIZE - 1; i++) {
		r[i] = (u[i + 1] - u[i]) / 2.0;

		if (PRINT_FUNCTIONS)
			printf("La diferencia entre dos valores es %f\n", r[i]);
	}

}

void countPositiveValues() {
	k = 0;

	for (int i = 0; i < SIZE; i++) {
		if (w[i] >= 0.0)
			k++;
	}

	if (PRINT_FUNCTIONS)
		printf("El contador de numeros positivos es %d\n", k);
}

void Sub() {
	//inicalizacion del vector V
	float* v = (float *)malloc(sizeof(float) * (SIZE - 1));
	for (int i = 0; i < SIZE - 1; i++) {
		v[i] = k * r[i];
	}

	//codigo del programa
	s = (float *)malloc(sizeof(float) * (SIZE - 1));
	for (int i = 0; i < SIZE - 1; i++) {
		s[i] = v[i] - t[i];
		if (PRINT_FUNCTIONS)
			printf("La resta es %f\n", s[i]);
	 }
	//eliminar de  memoria el vector V
	removeVector(v);
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
	srand((unsigned)time(&ti)); // Información sobre srand https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm

	double times[TIMES];

	for (int j = 0; j < TIMES; j++) {
		times[j] = 0;			//aseguramos valores reales
		for (int i = 0; i < NTIMES; i++) {
			u = createVector();
			w = createVector();
			t = createVector();

			times[j] += timer(Dif2) + timer(countPositiveValues) + timer(Sub);

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