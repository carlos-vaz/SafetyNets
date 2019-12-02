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
	vector<seal::Ciphertext> *sum;
	int size_of_sum;
	clock_t t;
};

server_out server(vector<seal::Ciphertext>*, seal::SEALContext, auto, int);
void read_MNIST(float*, int);
void serverConv(int, int, int, vector<seal::Ciphertext>*, vector<seal::Plaintext>*, vector<seal::Ciphertext>*, seal::Evaluator);

int main()
{
	// Set encryption parameters and print them out
	seal::EncryptionParameters params;
	params.set_poly_modulus("1x^4096 + 1");
	params.set_coeff_modulus(seal::coeff_modulus_128(4096));
	params.set_plain_modulus(1 << 12);
	seal::SEALContext context(params);

	// Key Generation
	seal::KeyGenerator keygen(context);
	auto public_key = keygen.public_key();
	auto secret_key = keygen.secret_key();

	// Fractional Encoder, Encryptor, Evaluator, and Decryptor init
	seal::FractionalEncoder encoder(context.plain_modulus(), context.poly_modulus(), 64, 32, 3);
	seal::Encryptor encryptor(context, public_key);
	seal::Decryptor decryptor(context, secret_key);

	// Read MNIST image
	cout << "reading mnist image...\n";
	float img [784];
	read_MNIST(img, 17);

	// Encode and encrypt image (timed)
	cout << "encoding/encrypting image...\n";
	vector<seal::Ciphertext> encImg;
	clock_t encryptor_ticks = clock();
	for(int i=0; i<784; i++)
	{
		cout << "\r" << string(i/7.84, '#') << string(100-i/7.84, ' ') << "|||";
		encImg.emplace_back(params);
		encryptor.encrypt(encoder.encode(img[i]), encImg[i]);
	}
	encryptor_ticks = clock() - encryptor_ticks;
	cout << "address of first element: " << &encImg[0] << endl;
	cout << "address of  next element: " << &encImg[1] << endl;
	cout << "ciphertext size: " << sizeof(encImg[0]) << " per pixel" << endl;

	// Print noises in fresh ciphertexts
	cout << "Noise budget for each ciphertext: " << endl;
	for(int i=0; i<784; i++)
	{
		cout << decryptor.invariant_noise_budget(encImg[i]) << ", ";
	}
	cout << endl;

	// Send to server for computation
	server_out output = server(&encImg, context, public_key, 784);

	// Decrypt result
	float *sum = new float[output.size_of_sum];
	clock_t decrypt_ticks = clock();
	for(int i=0; i<output.size_of_sum; i++)
	{
		seal::Plaintext temp;
		decryptor.decrypt((*output.sum)[i], temp);
		sum[i] = encoder.decode(temp);
		cout << sum[i] << " ,";
	}
	cout << "\n" << endl;
	decrypt_ticks = clock() - decrypt_ticks;

	// Send convolution result to process 'compare'
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addrs;
	bzero((char*)&server_addrs, sizeof(server_addrs));
	server_addrs.sin_family = AF_INET;
	server_addrs.sin_port = htons(8081);
	inet_pton(AF_INET, "127.0.0.1", &server_addrs.sin_addr);
	if(connect(sock, (struct sockaddr*)&server_addrs, sizeof(server_addrs))<0)
		cout << "Error connecting to socket" << endl;
	if(write(sock, sum, 4*3920)<0)
		cout << "Error writing convolution output to socket" << endl;

	// Print noises after computations
	cout << "Noise budget for each ciphertext: " << endl;
	for(int i=0; i<output.size_of_sum; i++)
	{
		cout << decryptor.invariant_noise_budget((*output.sum)[i]) << ", ";
	}
	cout << "\n\n";

	// Free memory used by 'sum'
	delete sum;


	cout << "#########     TIMING SUMMARY     #########" << endl;
	cout << "time to encode & encrypt: " << (float)encryptor_ticks/CLOCKS_PER_SEC << " seconds" << endl;
	cout << "time for convolution layer 1: " << (float)output.t/CLOCKS_PER_SEC << " seconds" << endl;
	cout << "time for decryption: " << (float)decrypt_ticks/CLOCKS_PER_SEC << " seconds" << endl;


	return 0;
}

server_out server(vector<seal::Ciphertext> *encImg, seal::SEALContext context, auto public_key, int imgSize)
{
	cout << "########    SERVER     ########\n";
	server_out ret;
	
	// Initialize Evaluator, Encoder, Encryptor
	seal::Evaluator evaluator(context);
	seal::Encryptor sEncryptor(context, public_key);
	seal::FractionalEncoder sEncoder(context.plain_modulus(), context.poly_modulus(), 64, 32, 3);

	// Read and encode weights
	cout << "Reading and encoding weights" << endl;
	float weights [125];
	FILE *f = fopen("./W_conv1.ckpt.data-00000-of-00001", "rb");
	fread(weights, sizeof(float), 125, f);
	fclose(f);
	vector<seal::Plaintext> plainWeights;
	for(int i=0; i<125; i++)
	{
		plainWeights.emplace_back(sEncoder.encode(weights[i]));
	}

	// Create ciphertext vector for convolution output, fill with encrypted zeros
	vector<seal::Ciphertext> sum;
	seal::Ciphertext encZero;
	sEncryptor.encrypt(sEncoder.encode(0), encZero);
	for(int i=0; i<28*28*5; i++)
	{
		sum.emplace_back(encZero);
	}
	
	// Convolution layer 1 (timed)
	cout << "convolution layer 1...\n";
	clock_t t = clock();
	serverConv(1, 5, 28, encImg, &plainWeights, &sum, evaluator);
	t = clock() - t;
	
	ret.sum = &sum;
	ret.size_of_sum = 28*28*5;
	ret.t = t;
	return ret;
}

void serverConv(int D, int N, int W, vector<seal::Ciphertext> *img, vector<seal::Plaintext> *w, vector<seal::Ciphertext> *out, seal::Evaluator eval)
{
	int count = 0;
	for (int n=0; n<N; n++)
	for (int z=0; z<D; z++)
	for (int y=0; y<W; y++)
	for (int x=0; x<W; x++) 
		for (int yy=0; yy<5; yy++)
		for (int xx=0; xx<5; xx++)
		{
			if (((y ==   0) && ((yy == 0) || (yy == 1))) ||  
			((y ==   1) && ((yy == 0))) ||  
			((y == W-2) && ((yy == 4))) ||  
			((y == W-1) && ((yy == 3) || (yy == 4))) ||  
			((x ==   0) && ((xx == 0) || (xx == 1))) ||  
			((x ==   1) && ((xx == 0))) ||  
			((x == W-2) && ((xx == 4))) ||
			((x == W-1) && ((xx == 3) || (xx == 4))))
			{
				// Boundary, add 0 (do nothing)
			} 
			else
			{
				seal::Ciphertext temp;
				cout << "\r" << string(count/900, '#') << string(100-count++/900, ' ') << "|||";
				eval.multiply_plain( (*img).at(z*W*W+(y-2+yy)*W+(x-2+xx)),  (*w).at(yy*D*5*N+xx*D*N+z*N+n),  temp   );
				eval.add( (*out)[n*W*W+y*W+x], temp);
			}
		}
	cout << "serverConv done" << endl;
}

void read_MNIST(float *img, int img_to_extract)
{
	ifstream mnist("./t10k-images-idx3-ubyte", ios::binary);
	if(mnist.is_open())
	{
		cout << "mnist is open...\n";
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

		cout << "extracting image...\n";
		// Extract image
		for(int i=0; i<784; i++)
		{
			unsigned char temp;
			mnist.read((char*)&temp, sizeof(unsigned char));
			img[i] = (float)temp;
		}
	}
	else
		printf("MNIST image open error!\n");
}
