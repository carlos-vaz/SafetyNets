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

struct server_out {
	vector<seal::Ciphertext> sum;
	int num_filters;
	int size_per_filter;
	clock_t t;
};

void read_MNIST(int64_t*, int);
server_out server(seal::Ciphertext, seal::SEALContext, auto, seal::Decryptor, seal::GaloisKeys, seal::EvaluationKeys);

int main()
{
	cout << "\n########    CLIENT     ########\n";

	// Set encryption parameters and print them out. Modulus T must be prime and congruent to 1 (mod 2*degree). 
	// Padded image is 1024 pixels, and for our batching scheme, we need at least twice this number on the top
	// row of the CRT Matrix. Thus, the row size must be >= 2048. This means the degree must be at least 4096. 
	seal::EncryptionParameters params;
	params.set_poly_modulus("1x^4096 + 1");
	//params.set_coeff_modulus(seal::coeff_modulus_128(4096));
	vector<seal::SmallModulus> small_moduli = {seal::small_mods_60bit(0), seal::small_mods_60bit(1),
	seal::small_mods_60bit(2),seal::small_mods_60bit(3), seal::small_mods_60bit(4), seal::small_mods_60bit(5), seal::small_mods_60bit(6), seal::small_mods_60bit(7)};
	params.set_coeff_modulus(small_moduli);
	params.set_plain_modulus(seal::small_mods_50bit(40));
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
	seal::Evaluator evaluator(context); 		// REMOVE

	// Read MNIST image
	cout << "Accessing MNIST database...\n";
	int64_t img [28*28];
	read_MNIST(img, 17);

//	for(int i=0; i<784; i++)
//		img[i] = 1;

	// Before encrypting, since we are CRT batching, we must pad 2 layers of 0's around the image as the 
	// server cannot do this. Note also that only the first 25% of pad_img contains data. The second 25% 
	// is reserved for Galois rotations, and the second half corresponds to the unused second row of the
	// CRT matrix. 
	//float pad_img[32*32*4];
	vector<int64_t> pad_img(32*32*4, 0);
	//memset(&pad_img, 0, sizeof(pad_img));
	for(int i=0; i<28; i++)
	{
		memcpy(&pad_img[64+2*(i+1)+30*i], &img[i*28], 28*sizeof(int64_t));
	}

/*
	// Print padded image to check
	for(int i=0; i<32*32; i++)
	{
		if(i%32==0)
			cout << endl;
		cout << pad_img[i] << " ";
	}
*/
	
	// Encode image with CRT and encrypt. Display noise budget of fresh ciphertext. 
	cout << "Encrypting..." << endl;
	seal::Plaintext plainImg;
	crtbuilder.compose(pad_img, plainImg);
	seal::Ciphertext encImg;
	encryptor.encrypt(plainImg, encImg);
	cout << "Noise budget in fresh ciphertext: " << decryptor.invariant_noise_budget(encImg) << " bits" << endl;


	// Send encoded image off to server
	server_out result = server(encImg, context, public_key, decryptor, gal_keys, eval_keys);

	// Decrypt server result
	cout << "\n########    CLIENT     ########\n";	
	cout << "Decrypting result..." << endl;
	vector<seal::Ciphertext> out = result.sum;
	cout << "Noise budget in fresh ciphertext: " << decryptor.invariant_noise_budget(out[0]) << " bits" << endl;
	int *conv_out = new int [result.num_filters*result.size_per_filter];
	for(int filter=0; filter<result.num_filters; filter++)
	{
		seal::Plaintext res;
		decryptor.decrypt(out[filter], res);
		vector<int64_t> filter_output;
		crtbuilder.decompose(res, filter_output);
		for(int i=0; i<result.size_per_filter; i++)
		{
			conv_out[i+filter*result.size_per_filter] = filter_output[i];
		}
	}

	// Display conv_out and timing for shits and giggles
	int num_printed = 0;
	int num_skipped = 0;
	for(int i=0; i<result.num_filters*result.size_per_filter; i++)
	{
		if(num_printed%28==0 && num_skipped==0)
			cout << endl;
		if(num_printed%784==0)
			cout << endl;
		if(num_printed%28==0 && num_skipped<4 && i>0)
		{
			num_skipped += 1;
			continue;
		}
		num_skipped = 0;
		cout << conv_out[i] << " ";
		num_printed += 1;
	}

	cout << endl; 
	cout << "\n########    TIMING SUMMARY     ########\n";	
	cout << "Time for convolution layer 1: " << (float)result.t/CLOCKS_PER_SEC << " seconds" << endl;

	delete conv_out; cout << endl; return 0;
}

void read_MNIST(int64_t *img, int img_to_extract)
{
	ifstream mnist("./t10k-images-idx3-ubyte", ios::binary);
	if(mnist.is_open())
	{
		int throw_away;
		mnist.read((char*)&throw_away, sizeof(int));
		mnist.read((char*)&throw_away, sizeof(int));
		mnist.read((char*)&throw_away, sizeof(int));
		mnist.read((char*)&throw_away, sizeof(int));

		// Read and throw away images until you get to the one you want to extract
		for(int i=0; i<img_to_extract*784; i++)
		{
			unsigned char throw_away;
			mnist.read((char*)&throw_away, sizeof(unsigned char));
		}

		cout << "Extracting image number "<< img_to_extract << "...\n";
		// Extract image
		for(int i=0; i<784; i++)
		{
			unsigned char temp;
			mnist.read((char*)&temp, sizeof(unsigned char));
			img[i] = (int64_t)temp;
		}
	}
	else
		printf("MNIST image open error!\n");
}

server_out server(seal::Ciphertext encImg, seal::SEALContext context, auto public_key, seal::Decryptor dec, seal::GaloisKeys gal_keys, seal::EvaluationKeys eval_keys)
{
	cout << "\n########    SERVER     ########" << endl;
	// Initialize Evaluator, Encoder, Encryptor
	seal::PolyCRTBuilder sCrtbuilder(context);
	seal::Evaluator evaluator(context);
	seal::Encryptor sEncryptor(context, public_key);

	// Determine the size of each of the two matrix rows
	int ROWSIZE = sCrtbuilder.slot_count()/2;

	// Read weights
	cout << "Reading and encoding weights (in real life, not necessary)..." << endl;
	float weights_float [125];
	FILE *f = fopen("./W_conv1.ckpt.data-00000-of-00001", "rb");
	fread(weights_float, sizeof(float), 125, f);
	fclose(f);

	// Unfortunately, CRT batching doesn't support floating point arithmetic. So we will multiply
	// each weight by 100 and round. The client will have to divide by 100 at the end. Accuracy 
	// is sacrificed for speed. 
	int weights_int [125];
	for(int i=0; i<125; i++)
	{
		weights_int[i] = (int)(1000*weights_float[i]);
//		if(weights_int[i]<0)
//			weights_int[i] *= -1;
	}

// display weights
	for(int i=0; i<125; i++)
	{
		if(i%5==0)
			cout << endl;
		if(i%25==0)
			cout << endl;
		cout << weights_int[i] << " ";
	}

	// Encode weights. In our batching scheme, each of the 125 weights is encoded into a 2-by-2048
	// matrix by simply inserting the weight into every slot of the first half of the top row. 
	vector<seal::Plaintext> weights;
	vector<int64_t> w(2*ROWSIZE, 0);
	for(int i=0; i<125; i++)
	{
		for(int ii=0; ii<2048; ii++)
			w[ii] = (int64_t)weights_int[i];
		seal::Plaintext temp;
		sCrtbuilder.compose(w, temp);
		weights.emplace_back(temp);
	}

	// Everything before this point can be pre-executed by sever. Timing begins now. 
	clock_t t = clock();

	// Generate 25 (size of sliding window) Galois automorphisms of the encrypted image. 
	cout << "Generating 25 image automorphisms..." << endl;
	vector<seal::Ciphertext> img_autom;
	for(int i=0; i<25; i++)
	{
		int rotate_amount = i%5 + (int)(i/5)*32;
		seal::Ciphertext temp(encImg);
		evaluator.rotate_rows(temp, rotate_amount, gal_keys);
		cout << "Noise budget in galois-ed ciphertext: " << dec.invariant_noise_budget(temp) << " bits" << endl;
		img_autom.emplace_back(temp);
	}
	cout << "Noise budget in [] ciphertext: " << dec.invariant_noise_budget(img_autom[0]) << " bits" << endl;

	// For each convolution filter (there are 5), perform convolution simply by multiplying each 
	// encoded weights plaintext with its corresponding image automorphism. This step takes only
	// 125 plain-to-cipher multiplications and 125 cipher-to-cipher additions. 
	cout << "Performing convolution..." << endl;
	vector<seal::Ciphertext> conv_out;
	for(int filter=0; filter<5; filter++)
	{
		// Isolate the weights for a particular filter
		vector<seal::Plaintext> filter_weights;
		for(int i=0; i<125; i++)
		{
			if(i%5==filter)
				filter_weights.emplace_back(weights[i]);
		}
		// Convolution for a particular filter
		vector<seal::Ciphertext> filter_img_autom;
		for(int i=0; i<25; i++)
		{
			seal::Ciphertext temp(img_autom[i]);
			filter_img_autom.emplace_back(temp);
			evaluator.multiply_plain(filter_img_autom[i], filter_weights[i]);
		}


		// Sum up all 25 image automorphism to obtain convolution output for a particular filter
		conv_out.emplace_back(filter_img_autom[0]);
		for(int i=1; i<25; i++)
		{
			evaluator.add(conv_out[filter], filter_img_autom[i]);
		}
	}

	// Square activation function
	for(int filter=0; filter<5; filter++)
	{
		evaluator.multiply(conv_out[filter], conv_out[filter]);
		evaluator.relinearize(conv_out[filter], eval_keys);
	}

	cout << "Noise budget in pre-return ciphertext: " << dec.invariant_noise_budget(conv_out[4]) << " bits" << endl;

	t = clock() - t;

	// Return conv_out to client
	server_out ret;
	ret.sum = conv_out;
	ret.num_filters = 5;
	ret.size_per_filter = 28*28+4*28;
	ret.t = t;
	return ret;
}
