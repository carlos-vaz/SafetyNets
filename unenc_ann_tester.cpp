#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <String.h>
#include "ann.h"
#include "tools.h"

using namespace std;

#define INPUT 784
#define HIDDEN 200
#define OUTPUT 10

int main()
{
	// Read MNIST single
	double img[INPUT];
	read_MNIST_single(img, 16);
	for(int i=0; i<INPUT; i++)
		img[i] = (double)((int)img[i]);

	display_image(img);

	// Read weights and binarize them in {-1,1}
	double w0 [INPUT*HIDDEN];
	double w1 [HIDDEN*OUTPUT];
	double b0 [HIDDEN];
	double b1 [OUTPUT];
	memset(b0, 0, sizeof(b0));
	memset(b1, 0, sizeof(b1));
	read_params(w0, INPUT*HIDDEN, "trained_Bweights_0");
	read_params(w1, HIDDEN*OUTPUT, "trained_Bweights_1");
	for(int i=0; i<INPUT*HIDDEN; i++)
		w0[i] = sign(w0[i]);
	for(int i=0; i<HIDDEN*OUTPUT; i++)
		w1[i] = sign(w1[i]);

	// Feed to neural network (you must comment out the sigmoid activation in ann.cpp)
	double out [OUTPUT];
	double hidden [HIDDEN];
	double z0 [HIDDEN];
	double z1 [OUTPUT];
	nn(img, w0, b0, w1, b1, hidden, out, z0, z1);

	// Display hidden activations
	for(int i=0; i<HIDDEN; i++)
		cout << hidden[i] << ", ";
	cout << "\n\n\n" << endl;

	// Display output activations
	for(int i=0; i<OUTPUT; i++)
		cout << out[i] << ", ";
	cout << endl;

	// Classification
	int64_t max = -1000000000;
	int index = -1;
	for(int i=0; i<OUTPUT; i++)
	{
		if( out[i]>max)
		{
			max = out[i];
			index = i;
		}
	}

	cout << "Predicted digit : " << index << "\n" << endl;
}
