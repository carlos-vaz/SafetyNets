#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "tools.h"
#include "ann.h"

using namespace std;

#define LRATE 0.00001
#define BATCH 5

#define INPUT	784
#define HIDDEN	200
#define OUTPUT	10

#define IMAGES 60000

void gradients1(double*, double*, double*, double*, double*, double*);
void gradients0(double*, double*, double*, double*, double*, double*, double*, double*);
void update(double*, double*, int, double);
int max_index(double*, int);

int main()
{
	// Read MNIST
	unsigned char *buf = new unsigned char [IMAGES*INPUT];
	unsigned char labels [IMAGES];
	read_MNIST_train(IMAGES, 784, buf, labels);

	// Read initial weights and biases
	double w0 [INPUT*HIDDEN];
	double w1 [HIDDEN*OUTPUT];
	double b0 [HIDDEN];
	double b1 [OUTPUT];

	read_params(w0, INPUT*HIDDEN, "trained_weights_0");
	read_params(w1, HIDDEN*OUTPUT, "trained_weights_1");
	memset(b0, 0, sizeof(b0));
	memset(b1, 0, sizeof(b1));

	double lrate = LRATE;

	for(int batch=0; batch<IMAGES/BATCH; batch++)
	{
		if(batch>5000)
			lrate = LRATE/10;
		if(batch>8000)
			lrate = LRATE/70;
		if(batch>10000)
			lrate = LRATE/100;

		double w0_bin [INPUT*HIDDEN];
		double w1_bin [HIDDEN*OUTPUT];

		binarize_deterministic(w0, w0_bin, INPUT*HIDDEN);
		binarize_deterministic(w1, w1_bin, HIDDEN*OUTPUT);

		// Process each batch
		double Gw1 [HIDDEN*OUTPUT];
		memset(Gw1, 0, sizeof(Gw1));
		double Gb1 [OUTPUT];
		memset(Gb1, 0, sizeof(Gb1));
		double *Gw0 = new double[INPUT*HIDDEN]();
		double Gb0 [HIDDEN];
		memset(Gb0, 0, sizeof(Gb0));

		int correct = 0;
		for(int i=0; i<BATCH; i++)
		{

			// Input img to NN
			double img [INPUT];
			double z0 [HIDDEN];
			double hidden [HIDDEN]; 
			double z1 [OUTPUT];
			double out [OUTPUT];

			for(int p=0; p<INPUT; p++)
				img[p] = (double)buf[BATCH*INPUT*batch + INPUT*i + p];

//			display_image(img);

			nn(img, w0_bin, b0, w1_bin, b1, hidden, out, z0, z1);

			// Determine desired output
			double desired [OUTPUT] = {0,0,0,0,0,0,0,0,0,0};
			desired[(int)labels[BATCH*batch + i]] = 1;

//		for(int t=0; t<10; t++)
//				cout << desired[t] << ",\t";
//			cout << endl;

			// Determine accuracy (for display purposes)
			if( max_index(out, 10)==labels[BATCH*batch + i] )
				correct++;


//			for(int t=0; t<10; t++)
//				cout << out[t] << ",\t";
//			cout << endl;


			// Determine gradients for weights/biases of output neurons
			double Gw1_tmp [HIDDEN*OUTPUT];
			memset(Gw1_tmp, 0, sizeof(Gw1_tmp));
			double Gb1_tmp [OUTPUT];
			memset(Gb1_tmp, 0, sizeof(Gb1_tmp));
			gradients1(out, desired, hidden, z1, Gw1_tmp, Gb1_tmp);
	
			// Determine gradients for w/b of hidden neurons
			double *Gw0_tmp = new double[INPUT*HIDDEN]();
			double Gb0_tmp [HIDDEN];
			memset(Gb0_tmp, 0, sizeof(Gb0_tmp));
			gradients0(out, desired, w1_bin, img, z0, z1, Gw0_tmp, Gb0_tmp);

			// Average gradients into batch gradient
			for(int t=0; t<HIDDEN*OUTPUT; t++)
				Gw1[t] += Gw1_tmp[t]/BATCH;
			for(int t=0; t<INPUT*HIDDEN; t++)
				Gw0[t] += Gw0_tmp[t]/BATCH;
			delete Gw0_tmp;
			for(int t=0; t<HIDDEN; t++)
				Gb1[t] += Gb1_tmp[t]/BATCH;
			for(int t=0; t<INPUT; t++)
				Gb0[t] += Gb0_tmp[t]/BATCH;
		}

		update(w0, Gw0, INPUT*HIDDEN, lrate);
//		update(b0, Gb0, HIDDEN, lrate);
		update(w1, Gw1, HIDDEN*OUTPUT, lrate);
//		update(b1, Gb1, OUTPUT, lrate);
		delete Gw0;


		cout << "Epoch " << batch << "\tCorrect out of " << BATCH << ": " << correct << endl;
	} 

	write_params(w0, INPUT*HIDDEN, "trained_PREbin_w0");
	write_params(w1, HIDDEN*OUTPUT, "trained_PREbin_w1");

	binarize_deterministic(w0, INPUT*HIDDEN);
	binarize_deterministic(w1, HIDDEN*OUTPUT);

	write_params(w0, INPUT*HIDDEN, "trained_Bweights_0");
	write_params(w1, HIDDEN*OUTPUT, "trained_Bweights_1");


	return 0;

}

void gradients1(double *out, double *ans, double *hidden, double *z1, double *Gw, double *Gb)
{
	// Store gradients in Gw[] and Gb[]
	for(int o=0; o<OUTPUT; o++)
		for(int h=0; h<HIDDEN; h++)
		{
	//		double zImpact = (out[o]-ans[o])/(2+exp(-1*pow(z1[o],2))+exp(pow(z1[o],2)));
			double zImpact = (out[o]-ans[o]);
			Gw[HIDDEN*o+h] = hidden[h]*zImpact;
			Gb[o] = 0;
		}
}

void gradients0(double *out, double *ans, double *w, double *input, double *z0, double *z1, double *Gw, double *Gb)
{
	// Store gradients in Gw[] and Gb[]
	for(int h=0; h<HIDDEN; h++)
	{
		double aImpact = 0;
		for(int o=0; o<OUTPUT; o++)
		{
	//		aImpact += (out[o]-ans[o])*w[OUTPUT*h+o]/(2+exp(-1*pow(z1[o],2))+exp(pow(z1[o],2)));
			aImpact += (out[o]-ans[o])*w[OUTPUT*h+o];
		}
	//	cout << "\t\t\taImpact: "<< aImpact<< endl;
		double zImpact = aImpact*2*z0[h];
		for(int i=0; i<INPUT; i++)
		{
			Gw[INPUT*h+i] = input[i]*zImpact;
			Gb[h] = 0;
		}
	}
}

void update(double* x, double* Gx, int size, double lrate)
{
	for(int i=0; i<size; i++)
		x[i] -= lrate*Gx[i];
}

int max_index(double* x, int size)
{
	int max = 0;
	int index = -1;
	for(int i=0; i<size; i++)
	{
		if(x[i] > max)
		{
			max = x[i];
			index = i;
		}
	}
	return index;
 }
