#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include <random>
#include <limits>
#include <time.h>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "seal/seal.h"

using namespace std;

int main()
{

// ########	CLIENT	    ##########
	seal::EncryptionParameters params_1;
	params_1.set_poly_modulus("1x^2048 + 1");
	params_1.set_plain_modulus(40960001);
	vector<seal::SmallModulus> small_moduli = {seal::small_mods_60bit(0), seal::small_mods_60bit(1),
	seal::small_mods_60bit(2)};
	params_1.set_coeff_modulus(small_moduli);
	seal::SEALContext context(params_1);
	
	// Save parameters to stream
	ofstream outfile("serialfile", ios_base::binary);
	params_1.save(outfile);
	outfile.close();
	

// ########	SERVER	    ##########
	ifstream infile("serialfile", ios_base::binary);
	seal::EncryptionParameters params_2;
	params_2.load(infile);
	infile.close();
	


	return 0;	
}

