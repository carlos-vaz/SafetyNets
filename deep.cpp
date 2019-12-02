#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "deep.h"

#define INPUT	784
#define H1	100
#define H2	50
#define OUTPUT	10

void nn(double *img, double *w0, double *w1, double *w2, double *out0, double *out1, double *out2, double *z0, double *z1, double *z2)
{
	// FC layer 1
	fc(img, w0, out0, z0, INPUT, H1);

	// square activation 1
	sq(out0, H1);

	// FC layer 2
	fc(out0, w1, out1, z1, H1, H2);

	// square activation 2
	sq(out1, H2);

	// FC layer 3
	fc(out1, w2, out2, z2, H2, OUTPUT);
	
	// sigmoid activation
	sigm(out2, OUTPUT);
}

void fc(double* in, double *w, double* out, double* z, int NUM_IN, int NUM_OUT)
{
	for(int o_neurs=0; o_neurs<NUM_OUT; o_neurs++)
	{
		out[o_neurs] = 0;
		for(int i_neurs=0; i_neurs<NUM_IN; i_neurs++)
		{
			out[o_neurs] += in[i_neurs] * w[o_neurs*NUM_IN + i_neurs];
		}
		z[o_neurs] = out[o_neurs];
	}
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
