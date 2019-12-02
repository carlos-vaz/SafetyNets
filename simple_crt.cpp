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


// TESTS CRT MULTIPLICATION OF VECTORS WITH NEGATIVE ELEMENTS

int main()
{
	cout << "\n########    CLIENT     ########\n";

	// Set encryption parameters and print them out. Modulus T must be prime and congruent to 1 (mod 2*degree). 
	// Padded image is 1024 pixels, and for our batching scheme, we need at least twice this number on the top
	// row of the CRT Matrix. Thus, the row size must be >= 2048. This means the degree must be at least 4096. 
	seal::EncryptionParameters params;
	params.set_poly_modulus("1x^4096 + 1");
	params.set_coeff_modulus(seal::coeff_modulus_128(4096));
	params.set_plain_modulus(40961);
	seal::SEALContext context(params);

	// Key Generation
	seal::KeyGenerator keygen(context);
	auto public_key = keygen.public_key();
	auto secret_key = keygen.secret_key();
	seal::GaloisKeys gal_keys;
	keygen.generate_galois_keys(10, gal_keys);

	// CRT encoder, encryptor, & decryptor init
	seal::PolyCRTBuilder crtbuilder(context); 
	seal::Encryptor encryptor(context, public_key);
	seal::Decryptor decryptor(context, secret_key);
	seal::Evaluator evaluator(context);

	return 0;
}
