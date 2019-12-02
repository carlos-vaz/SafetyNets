#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <String.h>
#include "tools.h"
#include "ann.h"

#define INPUT	784
#define HIDDEN	200
#define OUTPUT	10

#define IMAGES 10000

using namespace std;

int main()
{
	// Read weights and biases
	double w0 [INPUT*HIDDEN];
	double w1 [HIDDEN*OUTPUT];
	double b0 [HIDDEN];
	double b1 [OUTPUT];
	read_params(w0, INPUT*HIDDEN, "trained_Bweights_0");
	read_params(w1, HIDDEN*OUTPUT, "trained_Bweights_1");

//	binarize_deterministic(w0, INPUT*HIDDEN);
//	binarize_deterministic(w1, HIDDEN*OUTPUT);
	memset(b0, 0, sizeof(b0));
	memset(b1, 0, sizeof(b1));


	// Read MNIST images and labels
	unsigned char labels [IMAGES];
	unsigned char *images = new unsigned char [IMAGES*INPUT]();
	read_MNIST_test(IMAGES, 784, images, labels);

	// Deliver images one at a time to NN
	int num_correct = 0;
	for(int i=0; i<IMAGES; i++)
	{
		double single_img [28*28];
		for(int pix=0; pix<28*28; pix++)
			single_img[pix] = (double)images[28*28*i + pix];

		double out [OUTPUT];
		double hidden [HIDDEN];
		double z0 [HIDDEN];
		double z1 [OUTPUT];
		nn(single_img, w0, b0, w1, b1, hidden, out, z0, z1);
		
		// Classification
		int max = 0;
		int index = -1;
		for(int i=0; i<OUTPUT; i++)
		{
			if( out[i]>max)
			{
				max = out[i];
				index = i;
			}
		}

		// Test for accuracy
		if( index==labels[i] )
			num_correct++;
	}

	cout << "Number correct: " << num_correct << " \t(" << (float)(num_correct*100)/(float)IMAGES << "%% accuracy)" << endl;

	return 0;
}
