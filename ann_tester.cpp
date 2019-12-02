#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "ann.h"
#include "tools.h"

using namespace std;

#define INPUT 784
#define HIDDEN 100
#define OUTPUT 10

#define IMAGES 60000

int main()
{
	// Read MNIST
	unsigned char *images = new unsigned char [IMAGES*INPUT];
	unsigned char labels [IMAGES];
	read_MNIST_train(IMAGES, 784, images, labels);

	// Read weights and biases
	double w0 [INPUT*HIDDEN];
	double w1 [HIDDEN*OUTPUT];
	double b0 [HIDDEN];
	double b1 [OUTPUT];
	read_params(w0, INPUT*HIDDEN, "weights_0");
	read_params(w1, HIDDEN*OUTPUT, "weights_1");
	read_params(b0, HIDDEN, "biases_0");
	read_params(b1, OUTPUT, "biases_1");

	for(int i=0; i<100; i++)
	{
		double img [INPUT];
		double z0 [HIDDEN];
		double hidden [HIDDEN]; 
		double z1 [OUTPUT];
		double out [OUTPUT];

		for(int p=0; p<INPUT; p++)
			img[p] = (double)images[INPUT*i + p];

		display_image(img);

		nn(img, w0, b0, w1, b1, hidden, out, z0, z1);

		for(int d=0; d<10; d++)
			cout << out[d] << ",\t";
		cout << endl;
	}

	
}
