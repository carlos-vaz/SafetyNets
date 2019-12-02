#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <math.h>

#define INPUT 	784
#define	H1 	100
#define H2	50
#define OUTPUT 	10

using namespace std;

int main()
{
	ofstream w0;
	ofstream w1;
	ofstream w2;

	w0.open("deep_w0", ofstream::out | ofstream::trunc);
	w1.open("deep_w1", ofstream::out | ofstream::trunc);
	w2.open("deep_w2", ofstream::out | ofstream::trunc);

	// Create random floats from -1 to 1 using denominator RAND_MAX defined in <cstdlib>
	for(int i=0; i<INPUT*1; i++)
	{
		float num = (float)rand();
		float sign = (float)(rand() % 2) + 1;
		float r = (num/RAND_MAX) * (float)pow(-1, sign) ;
		w0 << r << "\n";
	}
	for(int i=0; i<H1*H2; i++)
	{
		float num = (float)rand();
		float sign = (float)(rand() % 2) + 1;
		float r = (num/RAND_MAX) * (float)pow(-1, sign) ;
		w1 << r << "\n";
	}
	for(int i=0; i<H2*OUTPUT; i++)
	{
		float num = (float)rand();
		float sign = (float)(rand() % 2) + 1;
		float r = (num/RAND_MAX) * (float)pow(-1, sign) ;
		w2 << r << "\n";
	}

	w0.close();
	w1.close();
	w2.close();

	return 0;
}
