#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <String.h>
#include "tools.h"

using namespace std;

#define INPUT 784
#define HIDDEN 200
#define OUTPUT 10

int main()
{
	ofstream outputA("trained_Bweights_0_flipped");
	ofstream outputB("trained_Bweights_1_flipped");

	double w0 [INPUT*HIDDEN];
	double w1 [HIDDEN*OUTPUT];
	read_params(w0, INPUT*HIDDEN, "trained_Bweights_0");
	read_params(w1, HIDDEN*OUTPUT, "trained_Bweights_1");

	for(int memb=0; memb<HIDDEN; memb++)
		for(int fam=0; fam<INPUT; fam++)
			outputA << w0[HIDDEN*fam+memb] << "\n";

	for(int memb=0; memb<OUTPUT; memb++)
		for(int fam=0; fam<HIDDEN; fam++)
			outputB << w1[OUTPUT*fam+memb] << "\n";

	outputA.close();
	outputB.close();



	return 0;
}
