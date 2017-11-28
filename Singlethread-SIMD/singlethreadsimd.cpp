// Single thread application with SIMD extensions

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
#define NTIMES						200								// Number of repetitions to get suitable times
#define SIZE						(1024*1024)						// Number of elements in the array
#define GET_VARIABLE_NAME(Variable)	(#Variable)
#define NUMBER_FLOAT				sizeof(__m256) / sizeof(float)
#define PRINT_FUNCTIONS				false
#define PRINT_TIMER_FUNCTIONS		false
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

float* createVector() {
	float* vector = (float *)_aligned_malloc(SIZE * sizeof(float), sizeof(__m256i));

	for (int i = 0; i < SIZE; i++) {
		float r = -1 + 2 * float((double)rand() / (double)(RAND_MAX)); // Aquí se explica por qué hacerlo con double: https://stackoverflow.com/questions/13408990/how-to-generate-random-float-number-in-c#comment56659626_13409133
		vector[i] = r;
	}
	return vector;
}

//eliminar vector
void removeVector(float* vector) {
	_aligned_free(vector);
}

void Dif2() {
	r = (float *)_aligned_malloc((SIZE-1) * sizeof(float), sizeof(__m256i));
	//Inicializamos una variable con el valor 2
	__m256 number2 = _mm256_set_ps(2, 2, 2, 2, 2, 2, 2, 2);

	for (int i = 0; i < (SIZE - 1) / NUMBER_FLOAT; i++) {
		__m256 valuei = *(__m256 *)&u[(i+1) * NUMBER_FLOAT];
		__m256 valuei_minus_1 = *(__m256 *)&u[i * NUMBER_FLOAT];
		__m256 value = _mm256_sub_ps(valuei, valuei_minus_1);
		value = _mm256_div_ps(value, number2);

		float *p = (float*)&value;							// Pointer p points to the first 32 integer in the packet
		for (int j = 0; j < NUMBER_FLOAT; j++) {
			r[i*NUMBER_FLOAT + j] = *p;
			p++;
			if (PRINT_FUNCTIONS)
				printf("La diferencia entre dos valores es %f\n", r[i+j]);
		}
	}
}

void countPositiveValues() {
	k = 0;

	__m256 mask = _mm256_set_ps(0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000);
	//Calculate count
	for (int j = 0; j < SIZE / NUMBER_FLOAT; j++) {
		__m256 value = *(__m256*)&w[j * NUMBER_FLOAT];
		__m256 and = _mm256_and_ps(value, mask);	// mascara para mirar el bit mas significativo

		float *p = (float*)&and;
		for (int i = 0; i < NUMBER_FLOAT; i++) {
			if (*(p+i) != 0x80000000) {
				k++;
			}
		}
		
	}
	if (PRINT_FUNCTIONS)
		printf("El contador de numeros positivos es %d\n", k);

}

//esta mal
void Sub() {
	//inicalizacion del vector V
	unsigned int index = 0;
	float* v = (float *)_aligned_malloc((SIZE - 1) * sizeof(float), sizeof(__m256i));
	__m256 kIntrisincs = _mm256_set_ps((float)k, (float)k, (float)k, (float)k, (float)k, (float)k, (float)k, (float)k);
	for (int i = 0; i < (SIZE - 1)/ NUMBER_FLOAT; i++) {
		__m256 value = *(__m256*)&r[i * NUMBER_FLOAT];
		__m256 mult = _mm256_mul_ps(kIntrisincs, value);

		float* p = (float*)&mult;
		for (int j = 0; j < NUMBER_FLOAT; j++) {
			v[i * NUMBER_FLOAT + j] = *(p+j);
		}

	}

	//codigo del programa
	index = 0;
	s = (float *)_aligned_malloc((SIZE - 1) * sizeof(float), sizeof(__m256i));
	for (int i = 0; i < (SIZE - 1) / NUMBER_FLOAT; i++) {
		__m256 valueV = *(__m256*)&v[i*NUMBER_FLOAT];
		__m256 valueT = *(__m256*)&t[i*NUMBER_FLOAT];
		__m256 sub = _mm256_sub_ps(valueV, valueT);

		//mal
		float* p = (float*)&sub;
		for (int j = 0; j < NUMBER_FLOAT; j++) {
			s[i * NUMBER_FLOAT + j] = *(p+j);

			if (PRINT_FUNCTIONS)
				printf("La resta es %f\n", s[i + j + index]);
		}
	}
	//eliminar de  memoria el vector V
	removeVector(v);
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
	if (PRINT_TIMER_FUNCTIONS)
		printf("Elapsed time in seconds: %f\n", dElapsedTimeS);
	// Return the elapsed time if it'll util
	return dElapsedTimeS;
}

void generateFile(double* times, double average, double std_deviation) {
	string nameFile = "times_" + to_string(average) + "_" + to_string(std_deviation) + ".csv";
	ofstream archivo(nameFile);

	for (int i = 0; i < NTIMES; i++) {
		archivo << times[i];
		archivo << ";";
	}
	archivo.close();
}

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

	/*free(times);*/
}