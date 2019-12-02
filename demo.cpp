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
#include <iterator>
#include "tools.h"
#include "seal/seal.h"

using namespace std;

#define INPUT	784
#define HIDDEN	200
#define OUTPUT	10

void server_prepare(seal::SEALContext context, vector<seal::Plaintext> *w0, vector<seal::Plaintext> *w1);
clock_t server(seal::Ciphertext*, seal::SEALContext, vector<seal::Ciphertext>*, seal::EvaluationKeys, seal::GaloisKeys, vector<seal::Plaintext>*, vector<seal::Plaintext>*);

int main() 
{
	cout << "\n########    CLIENT     ########\n";

	// Set encryption parameters and print them out. Modulus T must be prime and congruent to 1 (mod 2*degree). 
	seal::EncryptionParameters params;
	params.set_poly_modulus("1x^2048 + 1");
	params.set_plain_modulus(409600001);
	vector<seal::SmallModulus> small_moduli = {seal::small_mods_60bit(0), seal::small_mods_60bit(1),
	seal::small_mods_60bit(2)};
	params.set_coeff_modulus(small_moduli);
	seal::SEALContext context(params);

	// Key Generation
	seal::KeyGenerator keygen(context);
	auto public_key = keygen.public_key();
	auto secret_key = keygen.secret_key();
	seal::GaloisKeys gal_keys;
	seal::EvaluationKeys eval_keys;
	keygen.generate_galois_keys(30, gal_keys);
	keygen.generate_evaluation_keys(16, eval_keys);

	// CRT encoder, encryptor, & decryptor init
	seal::PolyCRTBuilder crtbuilder(context); 
	seal::Encryptor encryptor(context, public_key);
	seal::Decryptor decryptor(context, secret_key);

	// Server prepares in advanced
	vector<seal::Plaintext> w0;
	vector<seal::Plaintext> w1;
	server_prepare(context, &w0, &w1);

	// Check image file for new image
	// Create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0); 
	if(sock<=0)
		std::cout << "Error with socket creation" << std::endl;
	// Bind socket to port
	struct sockaddr_in my_addrs, cli_addrs;
	bzero((char*)&my_addrs, sizeof(my_addrs));
	my_addrs.sin_family = AF_INET;
	my_addrs.sin_addr.s_addr = INADDR_ANY;
	my_addrs.sin_port = htons(8081);
	int bind_status = bind(sock, (struct sockaddr*)&my_addrs, sizeof(my_addrs));
	if(bind_status<0)		
		std::cout << "Error binding socket to localhost:8081" << std::endl;
	// Enable listening for incoming connections (accept max 2 connections)
	if(listen(sock, 2)<0)
		std::cout << "Error listening on port 8081" << std::endl;
	socklen_t clilen = sizeof(cli_addrs);
	bzero((char*)&cli_addrs, clilen);
	string recieved;
while(1) {
	int new_sock = accept(sock, (struct sockaddr*)&cli_addrs, &clilen);
	if(read(new_sock, &recieved, 1) == 1)
	{
	// Received command, read image file
	vector<vector<int64_t>> img;
//	putMNIST_in_Image_file(rand()%99, "./drawing/image");
	read_SingleImageFile(&img, HIDDEN, INPUT, "./drawing/image");

	// FOR DISPLAY ONLY ##########################
	unsigned char imagen [INPUT];
	for(int i=0; i<INPUT; i++)
		imagen[i] = (unsigned char)(img[i])[0];
	display_image(imagen);
	// ###########################################

	// Encode and encrypt image
	cout << "ENCRYPTING IMAGE..." << "\n" << endl;
	clock_t t = clock();
	vector<seal::Ciphertext> cipherImg;
	for(int i=0; i<INPUT; i++)
	{
		seal::Plaintext plaintemp;
		seal::Ciphertext ciphertemp;
		crtbuilder.compose(img[i], plaintemp);
		encryptor.encrypt(plaintemp, ciphertemp);
		cipherImg.emplace_back(ciphertemp);
	}
	t = clock() - t;

	cout << "Noise budget in fresh ciphertext: " << decryptor.invariant_noise_budget(cipherImg[0]) << " bits" << endl;

	// Send image to server
	seal::Ciphertext result;
	clock_t eval_t = server(&result, context, &cipherImg, eval_keys, gal_keys, &w0, &w1);


	clock_t decrypt_t = clock();
	seal::Plaintext result_plain;
	vector<int64_t> output;
	decryptor.decrypt(result, result_plain);
	crtbuilder.decompose(result_plain, output);
	decrypt_t = clock() - decrypt_t;

	cout << "\n########    CLIENT     ########\n";

	// Classification
	int maximal_index = -1;
	int64_t max = -10000000000;
	for(int i=0; i<OUTPUT; i++)
	{
		if(output[i] > max)
		{
			max = output[i];
			maximal_index = i;
		}
	}

	for(int i=0; i<OUTPUT; i++)
		cout << output[i] << ", ";
	cout << "\n" << endl;

	cout << "Predicted digit: " << maximal_index << "\n" << endl;
	cout << "Noise budget used cipher: " << decryptor.invariant_noise_budget(result) << " bits" << endl;

	cout << "\n########    TIMING RESULTS    ########\n";
	cout << "Time for encoding & encryption: " << (float)t/CLOCKS_PER_SEC << " seconds" << endl;
	cout << "Time for computation: " << (float)eval_t/CLOCKS_PER_SEC << " seconds" << endl;
	cout << "Time for decryption: " << (float)decrypt_t/CLOCKS_PER_SEC << " seconds" << endl;
}
}
	

	return 0;	
}

void server_prepare(seal::SEALContext context, vector<seal::Plaintext> *w0, vector<seal::Plaintext> *w1)
{
	// Initialize CRT Batcher and Evaluator
	seal::PolyCRTBuilder sCrtbuilder(context);
	seal::Evaluator evaluator(context);

	// Read and encode binary weights (most significant step to be done in advance)
	vector<vector<int64_t>> store0;
	vector<vector<int64_t>> store1;
//	vector<seal::Plaintext> w0;
//	vector<seal::Plaintext> w1;
	read_binaryWeightChunks(&store0, HIDDEN, INPUT, "trained_Bweights_0");
	read_binaryWeightChunksShmeared(&store1, OUTPUT, HIDDEN+OUTPUT-1, "trained_Bweights_1");
	for(int i=0; i<INPUT; i++)
	{
		seal::Plaintext w;
		sCrtbuilder.compose(store0[i], w);
		(*w0).emplace_back(w);
	}
	for(int i=0; i<HIDDEN+OUTPUT-1; i++)
	{
		seal::Plaintext w;
		sCrtbuilder.compose(store1[i], w);
		(*w1).emplace_back(w);
	}
}

clock_t server(seal::Ciphertext* result, seal::SEALContext context, vector<seal::Ciphertext> *img, seal::EvaluationKeys eval_keys, seal::GaloisKeys gal_keys, vector<seal::Plaintext> *w0, vector<seal::Plaintext> *w1)
{
	cout << "\n########    SERVER     ########\n";

	// Initialize CRT Batcher and Evaluator
	seal::PolyCRTBuilder sCrtbuilder(context);
	seal::Evaluator evaluator(context);
/*
	// Read and encode binary weights (most significant step to be done in advance)
	vector<vector<int64_t>> store0;
	vector<vector<int64_t>> store1;
	vector<seal::Plaintext> w0;
	vector<seal::Plaintext> w1;
	read_binaryWeightChunks(&store0, HIDDEN, INPUT, "trained_Bweights_0");
	read_binaryWeightChunksShmeared(&store1, OUTPUT, HIDDEN+OUTPUT-1, "trained_Bweights_1");
	for(int i=0; i<INPUT; i++)
	{
		seal::Plaintext w;
		sCrtbuilder.compose(store0[i], w);
		w0.emplace_back(w);
	}

	for(int i=0; i<HIDDEN+OUTPUT-1; i++)
	{
		seal::Plaintext w;
		sCrtbuilder.compose(store1[i], w);
		w1.emplace_back(w);
	}
*/
	clock_t t = clock();

/*
	// DECRYPT AND DISPLAY WEIGHTS_1 ###################
	for(int i=0; i<100; i++) {
		seal::Plaintext see_plain;
		vector<int64_t> see;
	//	decryptor.decrypt(w0[i], see_plain);
		sCrtbuilder.decompose(w1[i], see);
		for(int j=0; j<OUTPUT+5; j++)
			cout << see[j] << ", ";
		cout << "\n\n" << endl;
	}
	// ##########################################################
*/

	// FC layer 1
	seal::Ciphertext sum0;
	evaluator.multiply_plain((*img)[0], (*w0)[0], sum0);
	for(int i=1; i<INPUT; i++)
	{
		seal::Ciphertext product;
		evaluator.multiply_plain((*img)[i], (*w0)[i], product);
		evaluator.add(sum0, product);
	}

	// Square activation
	evaluator.multiply(sum0, sum0);
	evaluator.relinearize(sum0, eval_keys);

/*
	// DECRYPT AND DISPLAY HIDDEN ACTIVATIONS ###################
	seal::Plaintext see_plain;
	vector<int64_t> see;
	decryptor.decrypt(sum0, see_plain);
	sCrtbuilder.decompose(see_plain, see);
	for(int i=0; i<HIDDEN; i++)
		cout << see[i] << ", ";
	cout << "\n\n" << endl;
	// ##########################################################
*/

	// FC layer 2
	// First, rotate previous activation HIDDEN times starting from (OUTPUT-1 right) to (HIDDEN-1 left). 
	// Then, multiply each of these ciphertexts (families) by their corresponding weights (family members)
	// and add them all together. Add all HIDDEN+OUTPUT-1 of these together. Only the first 10 elements of the
	// result matter. 
	seal::Ciphertext sum_autom(sum0);
	evaluator.rotate_rows(sum_autom, 1-OUTPUT, gal_keys);
	evaluator.multiply_plain(sum_autom, (*w1)[0], *result);
	for(int i=1; i<HIDDEN+OUTPUT-1; i++)
	{
		int rotate_amount = i - (OUTPUT-1);
		seal::Ciphertext sum_autom(sum0);
		evaluator.rotate_rows(sum_autom, rotate_amount, gal_keys);
		evaluator.multiply_plain(sum_autom, (*w1)[i]);
		evaluator.add(*result, sum_autom);
	}
	t = clock() - t;
	return t;
}
