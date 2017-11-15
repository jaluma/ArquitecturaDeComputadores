// Single thread application with SIMD extensions

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

#define NTIMES						10								// Number of repetitions to get suitable times
#define SIZE						(1024*1024)						// Number of elements in the array
#define GET_VARIABLE_NAME(Variable)	(#Variable)
#define PRINT_FUNCTIONS				true
#define PRINT_TIMER					false

// Definicion de funciones a usar en el main
void createVector();
void removeVector();
void Sub(int* vector);
void Dif2(int* vector);
void CountPositiveValues(int* vector);

// Calculo de media y desviación
double timer(void(*function)(int*));
void generateFile(double* times, double average, double std_deviation, string nameFunction);
void calculate_average_deviation(void(*function)(int*));

// Timer
LARGE_INTEGER frequency;
LARGE_INTEGER tStart;
LARGE_INTEGER tEnd;
double dElapsedTimeS;
int* vector;	// int[] vector;

int main() {
	// funcion Sub
	printf("Ejecutando Funcion Sub...\n");
	calculate_average_deviation(Sub);
	// funcion Dif
	printf("Ejecutando Funcion Dif2...\n");
	calculate_average_deviation(Dif2);
	// funcion CuntPositiveValues
	printf("Ejecutando Funcion CountPositiveValues...\n");
	calculate_average_deviation(CountPositiveValues);
}

// Crear vector
void createVector() {
	vector = (int *)_aligned_malloc(SIZE * sizeof(int), sizeof(__m256i));

	for (int i = 0; i < SIZE; i++) {
		vector[i] = (rand() % 3) - 1;	// rango de (0,2) - 1 ==> (-1, 1)
	}
}

//eliminar vector
void removeVector() {
	_aligned_free(vector);
}

void calculate_average_deviation(void(*function)(int*)) {
	double times[NTIMES];
	double average, variance, std_deviation, sum = 0, sum1 = 0;

	for (int i = 0; i < NTIMES; i++) {
		createVector();
		times[i] = timer(function);
		sum += times[i];
		removeVector();
	}

	average = sum / (double)NTIMES;

	for (int i = 0; i < NTIMES; i++) {
		sum1 = sum1 + pow((times[i] - average), 2);
	}

	variance = sum1 / (double)NTIMES;
	std_deviation = sqrt(variance);

	printf("La media de tiempos es: %f\r\n", average);
	printf("La desviacion tipica de tiempos es: %f\r\n", std_deviation);

	generateFile(times, average, std_deviation, GET_VARIABLE_NAME(function));
}

void Sub(int* vector) {
	int sub = 0;
	/*__m256i sub4 = _mm256_load_epi32(&vector[0]);*/				 // 512 bits type, storing sixteen 32 bit integers
	__m256i sub4 = *(__m256i*)vector;

	// Calculate the sub
	for (int j = 1; j < SIZE / 8; j++) {
		sub4 = _mm256_sub_epi32(sub4, *(__m256i *)&vector[j * 8]);

		int *p = (int*)&sub4;										// Pointer p points to the first 32 integer in the packet
		for (int i = 0; i < 256 / 32; i++) {
			sub += *(p + i);										// Ahora se suman los valores de las restas calculadas
		}
	}
	// Print the sum
	if (PRINT_FUNCTIONS)
		printf("La resta es %d\n", sub);
}

void CountPositiveValues(int* vector) {
	unsigned int count = 0, i = 0, maskInteger = 0x80000000;
	__m256i mask = _mm256_set1_epi32(maskInteger);
	//Calculate count
	for (int j = 0; j < SIZE / 256; j++) {
		__m256i value = *(__m256i*)&vector[j * 8];			// mal, no se arreglarlo. ellos lo tienen igual
		__m256i and = _mm256_and_si256(value, mask);	// mascara para mirar el bit mas significativo

		int *p = (int*)&and;

		if (*p != 0x80000000) {
			count++;
		}
	}
	if (PRINT_FUNCTIONS)
		printf("El contador de numeros positivos es %d\n", count);
}

void Dif2(int* vector) {
	float valueFloat = 0;
	int i = 0;
	__m256 value = *(__m256 *)_aligned_malloc(SIZE * sizeof(int), sizeof(__m256i));// 512 bits type, storing sixteen 32 bit integers

																				   // Convierto el vector a float. Misma referencia para luego dejar libre la misma dirección de memoria
	float* vectorFloat = (float*)vector;
	for (int j = 0; j < SIZE; j++) {
		vectorFloat[j] = (float)vector[j];
	}

	//Inicializamos una variable con el valor 2
	__m256 number2 = _mm256_set_ps(2, 2, 2, 2, 2, 2, 2, 2);

	//Codigo
	for (i = 1; i < SIZE / 8; i++) {
		__m256 valuei = *(__m256 *)&vector[i * 8];
		__m256 valueiplus1 = *(__m256 *)&vector[(i - 1) * 8];
		value = _mm256_sub_ps(valuei, valueiplus1);
		value = _mm256_div_ps(value, number2);

		float *p = (float*)&value;							// Pointer p points to the first 32 integer in the packet
		for (int i = 0; i < 256 / 32; i++) {
			/*valueFloat += (*(p + i) + *(p + i - 1) )/ 2;*/
			valueFloat += *(p + i);
		}
	}
	if (PRINT_FUNCTIONS)
		printf("La diferencia entre dos valores es %02.f\n", valueFloat);
}

// Codigo para calcular tiempos

double timer(void(*function)(int*)) {
	// Get clock frequency in Hz
	QueryPerformanceFrequency(&frequency);
	// Get initial clock count
	QueryPerformanceCounter(&tStart);
	// Code to be measured
	function(vector);
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

void generateFile(double* times, double average, double std_deviation, string nameFuction) {
	string nameFile = "times_" + nameFuction + "_" + to_string(average) + "_" + to_string(std_deviation) + ".csv";
	ofstream archivo(nameFile);

	for (int i = 0; i < NTIMES; i++) {
		archivo << times[i];
		archivo << "\n";
	}

	archivo.close();
}