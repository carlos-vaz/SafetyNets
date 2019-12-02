#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "tools.h"

using namespace std;

int main()
{
	double prob_of_1 = 0.17987234;

	for(int i=0; i<100; i++)
		cout << probability(prob_of_1) << ", ";
	cout << endl;
}
