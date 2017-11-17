// Single thread application

#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
// Include required header files

#define NTIMES            200   // Number of repetitions to get suitable times
#define SIZE      (1024*1024)   // Number of elements in the array

// Timer
LARGE_INTEGER frequency;
LARGE_INTEGER tStart;
LARGE_INTEGER tEnd;
double dElapsedTimeS;
float* vector;	// int[] vector;

void createVector();
void removeVector();
double timer(void(*function)(void));
void generateFile(double* times, double average, double std_deviation);

// Comentario main Lino
int main() {
	double times[NTIMES];
	double average, variance, std_deviation, sum = 0, sum1 = 0;

	for (int i = 0; i < NTIMES; i++) {
		times[i] = timer(createVector);
		sum += times[i];
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

	removeVector();
}

void createVector() {
	vector = (float *)malloc(SIZE * sizeof(float));

	for (int i = 0; i < SIZE; i++) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = 1 - (-1);
		float r = random * diff;
		vector[i] = (-1) + r;;	// rango de (0,2) - 1 ==> (-1, 1)
	}
}

//eliminar vector
void removeVector() {
	free(vector);
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
