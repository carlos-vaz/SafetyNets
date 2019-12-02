#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "tools.h"
#include "deep.h"

using namespace std;

#define LRATE 0.00000000000001
#define BATCH 5

#define INPUT	784
#define H1	100
#define H2	50
#define OUTPUT	10

#define IMAGES 60000

void gradients2(double*, double*, double*, double*);
void gradients1(double*, double*, double*, double*, double*, double*);
void gradients0(double*, double*, double*, double*, double*, double*, double*, double*);
void update(double*, double*, int);
int max_index(double*, int);

int main()
{
	// Read MNIST
	unsigned char *buf = new unsigned char [IMAGES*INPUT];
	unsigned char labels [IMAGES];
	read_MNIST_train(IMAGES, 784, buf, labels);

	// Read initial weights and biases
	double w0 [INPUT*H1];
	double w1 [H1*H2];
	double w2 [H2*OUTPUT];
	read_params(w0, INPUT*H1, "deep_w0");
	read_params(w1, H1*H2, "deep_w1");
	read_params(w2, H2*OUTPUT, "deep_w2");

	// Process batches
	for(int batch=0; batch<IMAGES/BATCH; batch++)
	{
		// Create gradient vectors
		double *Gw0 = new double[INPUT*H1]();
		double *Gw1 = new double[H1*H2]();
		double Gw2 [H2*OUTPUT];
		memset(Gw2, 0, sizeof(Gw2));

		int correct = 0;
		for(int i=0; i<BATCH; i++)
		{

			// Prepare snapshot vectors for quicker training
			double img [INPUT];
			double z0 [H1];
			double hidden_1 [H1];
			double z1 [H2];
			double hidden_2 [H2];
			double z2 [OUTPUT];
			double out [OUTPUT];

			// Extract a single image
			for(int p=0; p<INPUT; p++)
				img[p] = (double)buf[BATCH*INPUT*batch + INPUT*i + p];

			// Determine desired output
			double desired [OUTPUT] = {0,0,0,0,0,0,0,0,0,0};
			desired[(int)labels[BATCH*batch + i]] = 1;

			// Deliver image to NN
			nn(img, w0, w1, w2, hidden_1, hidden_2, out, z0, z1, z2);

cout << "\t\t\t";
for(int ii=0; ii<10; ii++)
	cout << out[ii] << ", ";
cout << endl;

			// Determine accuracy (for display purposes)
			if( max_index(out, 10)==labels[BATCH*batch + i] )
				correct++;

			// Determine gradients for weights of output neurons
			double Gw2_tmp [H2*OUTPUT];
			memset(Gw2_tmp, 0, sizeof(Gw2_tmp));
			gradients2(out, desired, hidden_2, Gw2_tmp);
	
			// Determine gradients weights of hidden layer 2 neurons
			double *Gw1_tmp = new double[H1*H2]();
			gradients1(out, desired, w2, hidden_1, z1, Gw1_tmp);

			// Determine gradients weights of hidden layer 1 neurons
			double *Gw0_tmp = new double[INPUT*H1]();
			gradients0(out, desired, w1, w2, img, z0, z1, Gw0_tmp);

			// Average gradients into batch gradient and free their memories
			for(int t=0; t<H2*OUTPUT; t++)
				Gw2[t] += Gw2_tmp[t]/BATCH;
			for(int t=0; t<H1*H2; t++)
				Gw1[t] += Gw1_tmp[t]/BATCH;
			for(int t=0; t<INPUT*H1; t++)
				Gw0[t] += Gw0_tmp[t]/BATCH;
			delete Gw1_tmp;
			delete Gw0_tmp;
		}

		update(w0, Gw0, INPUT*H1);
		update(w1, Gw1, H1*H2);
		update(w2, Gw2, H2*OUTPUT);
		delete Gw1;
		delete Gw0;


		cout << "Epoch " << batch << "\tCorrect out of " << BATCH << ": " << correct << endl;
	} 

	write_params(w0, INPUT*H1, "trained_deep_w0");
	write_params(w1, H1*H2, "trained_deep_w1");
	write_params(w2, H2*OUTPUT, "trained_deep_w2");


	return 0;

}

void gradients2(double *out, double *ans, double *a_prev, double *Gw)
{
	// Store gradients for w2 in Gw[]
	for(int o=0; o<OUTPUT; o++)
		for(int h=0; h<H2; h++)
		{
			double zImpact = (out[o]-ans[o]);
			Gw[H2*o+h] = a_prev[h]*zImpact;
		}
}

void gradients1(double *out, double *ans, double *w2, double *a_prev, double *z1, double *Gw)
{
	// Store gradients for w1 in Gw[]
	for(int h=0; h<H2; h++)
	{
		double aImpact = 0;
		for(int o=0; o<OUTPUT; o++)
		{
			aImpact += (out[o]-ans[o])*w2[OUTPUT*h+o];
		}
		double zImpact = aImpact*2*z1[h];
		for(int i=0; i<H1; i++)
		{
			Gw[H1*h+i] = a_prev[i]*zImpact;
		}
	}
}

void gradients0(double* out, double *ans, double *w1, double *w2, double *input, double *z0, double *z1, double *Gw)
{
	// Store gradients for w0 in Gw[]
	for(int i=0; i<INPUT; i++)
	{
		for(int j=0; j<H1; j++)
		{

			double a0Impact = 0;
			for(int k=0; k<H2; k++)
			{
				double a1Impact = 0;
				for(int l=0; l<OUTPUT; l++)
				{
					a1Impact += w2[OUTPUT*k+l]*(out[l]-ans[l]);
				}
				a0Impact += w1[H2*j+k]*2*z1[k]*a1Impact;
			}
			Gw[H2*i+j] = input[i]*2*z0[j]*a0Impact;
		}
	}
}

void update(double* x, double* Gx, int size)
{
	for(int i=0; i<size; i++)
		x[i] -= LRATE*Gx[i];
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
