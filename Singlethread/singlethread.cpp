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
// Include required header files

#define TIMES						10
#define NTIMES						200							// Number of repetitions to get suitable times
#define SIZE						(1024*1024)					// Number of elements in the array
#define GET_VARIABLE_NAME(Variable)	(#Variable)
#define PRINT_FUNCTIONS				false
#define PRINT_TIMER_FUNCTION		false
#define PRINT_TIMER					true

// Timer
LARGE_INTEGER frequency;
LARGE_INTEGER tStart;
LARGE_INTEGER tEnd;
double dElapsedTimeS;

//definicion de los atributos a usar en las funciones
float* u;		//vector usado en Dif2
float* t;		//vector usado en Sub
float* w;		//vector usado en ContarPositivos																																							                                                                       
//atributos de return
float* r;			// vector resultante de op1
unsigned int k;		//numero de positivos op2
float* s;			// vector resultante de op3


//devuelve un vector de tamaño SIZE
float* createVector() {
	float* vector = (float *)malloc(sizeof(float) * SIZE);

	for (int i = 0; i < SIZE; i++) {
		//float random = ((float)rand()) / (float)RAND_MAX;
		//float diff = 1 - (-1);
		//float r = random * diff;
		//vector[i] = (-1) + r;	// rango de (0,2) - 1 ==> (-1, 1)

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
		s[i] = v[i] - u[i];
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

// genera un archivo *.csv con los tiempos que devuelve la funcion timer
void generateFile(double* times, double average, double std_deviation) {
	string nameFile = "times_" + to_string(average) + "_" + to_string(std_deviation) + ".csv";
	ofstream archivo(nameFile);

	for (int i = 0; i < NTIMES; i++) {
		archivo << times[i];
		archivo << ";";
	}
	archivo.close();
}

// funcion main del programa singleThread
int main() {
	time_t ti;
	srand((unsigned)time(&ti)); // Información sobre srand https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm

	double times[NTIMES];

	for (int j = 0; j < TIMES; j++) {
		double average, variance, std_deviation, sum = 0, sum1 = 0;

		for (int i = 0; i < NTIMES; i++) {
			u = createVector();
			w = createVector();
			t = createVector();

			times[i] = timer(Dif2) + timer(countPositiveValues) + timer(Sub);
			sum += times[i];

			if (PRINT_TIMER)
				printf("Elapsed time in seconds: %f\n", times[i]);

			removeVector(u);
			removeVector(w);
			removeVector(t);
			removeVector(s);
			removeVector(r);
		}

		average = sum / (double)NTIMES;

		for (int i = 0; i < NTIMES; i++) {
			sum1 = sum1 + pow((times[i] - average), 2);
		}

		variance = sum1 / (double)NTIMES;
		std_deviation = sqrt(variance);

		printf("La media de tiempos es: %f\r\n", average);
		printf("La desviacion tipica de tiempos es: %f\r\n", std_deviation);

		generateFile(times, average, std_deviation);
	}

	free(times);
}