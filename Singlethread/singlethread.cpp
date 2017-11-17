// Single thread application

#include <Windows.h>
#include <intrin.h>
#include "immintrin.h"
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
// Include required header files

#define NTIMES						1								// Number of repetitions to get suitable times
#define SIZE						1024/*(1024*1024)	*/					// Number of elements in the array
#define GET_VARIABLE_NAME(Variable)	(#Variable)
#define PRINT_FUNCTIONS				true
#define PRINT_TIMER					false

// Timer
LARGE_INTEGER frequency;
LARGE_INTEGER tStart;
LARGE_INTEGER tEnd;
double dElapsedTimeS;

float* u;
float* t;
float* w;
float* v;                                                                                                                                                                                                                                                                              

//atributos de return
float* r;			// vector resultante de op1
unsigned int k;		//numero de positivos op2
float* s;			// vector resultante de op3

float* createVector() {
	float* vector = (float *)malloc(sizeof(float) * SIZE);

	for (int i = 0; i < SIZE; i++) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = 1 - (-1);
		float r = random * diff;
		vector[i] = (-1) + r;	// rango de (0,2) - 1 ==> (-1, 1)
		/*vector[i] = 1;*/
	}
	return vector;
}

//eliminar vector
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
	v = (float *)malloc(sizeof(float) * (SIZE - 1));
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

	removeVector(v);
	removeVector(s);
}

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
	if (PRINT_TIMER)
		printf("Elapsed time in seconds: %f\n", dElapsedTimeS);
	// Return the elapsed time if it'll util
	return dElapsedTimeS;
}

void generateFile(double* times, double average, double std_deviation) {
	string nameFile = "times_" + to_string(average) + "_" + to_string(std_deviation) + ".csv";
	ofstream archivo(nameFile);

	for (int i = 0; i < NTIMES; i++) {
		archivo << times[i];
		archivo << "\n";
	}
	archivo.close();
}

// Comentario main Lino
int main() {
	double times[NTIMES];
	double average, variance, std_deviation, sum = 0, sum1 = 0;

	for (int i = 0; i < NTIMES; i++) {
		u = createVector();
		w = createVector();
		t = createVector();

		times[i] = timer(Dif2) + timer(countPositiveValues) + timer(Sub);

		sum += times[i];
		removeVector(u);
		removeVector(w);
		removeVector(t);
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