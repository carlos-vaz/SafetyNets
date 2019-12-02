#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "ann.h"

#define INPUT	784
#define HIDDEN	200
#define OUTPUT	10

void nn(double *img, double *w0, double *b0, double *w1, double *b1, double *out0, double *out1, double *z0, double *z1)
{
	// FC layer 1
	fc(img, w0, b0, out0, z0, INPUT, HIDDEN);

	// square activation 1
	sq(out0, HIDDEN);

	// FC layer 2
	fc(out0, w1, b1, out1, z1, HIDDEN, OUTPUT);

	// square activation 2
//	sq(out1, OUTPUT);
	
	// sigmoid activation
//	sigm(out1, OUTPUT);
}

void fc(double* in, double *w, double *b, double* out, double* z, int NUM_IN, int NUM_OUT)
{
	for(int o_neurs=0; o_neurs<NUM_OUT; o_neurs++)
	{
		out[o_neurs] = 0;
		for(int i_neurs=0; i_neurs<NUM_IN; i_neurs++)
		{
			out[o_neurs] += in[i_neurs] * w[o_neurs*NUM_IN + i_neurs];
			std::cout <<  w[o_neurs*NUM_IN + i_neurs] << ", ";
		}
		std::cout << "\n" << std::endl;
		out[o_neurs] += b[o_neurs];
		z[o_neurs] = out[o_neurs];
	}
	std::cout << "\n\n\n######## END ########\n\n\n" << std::endl;
}

void sq(double* inout, int size)
{
	for(int i=0; i<size; i++)
		inout[i] *= inout[i];
}

void sigm(double* inout, int size)
{
	for(int i=0; i<size; i++)
		inout[i] = 1 / (1 + exp(-1*inout[i]));
}
