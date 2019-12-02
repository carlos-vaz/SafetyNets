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
#include "tools.h"
#include "seal/seal.h"

using namespace std;

#define INPUT	784
#define HIDDEN	200
#define OUTPUT	10

void read_socket(int, unsigned char*);

int main()
{
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

	// Enable listening for incoming connections (accept max 2 connections at a time)
	if(listen(sock, 2)<0)
		std::cout << "Error listening on port 8081" << std::endl;


	// Wait for connections, accept when they arrive, and store messages
	socklen_t clilen = sizeof(cli_addrs);
	bzero((char*)&cli_addrs, clilen);
	int new_sock = accept(sock, (struct sockaddr*)&cli_addrs, &clilen);


	// Read message concerning number of bytes to read
	int bytes_to_read;
	read(new_sock, &bytes_to_read, sizeof(int));

	// Read message from client, store in appropriate buffer
	unsigned char *input_buffer = new unsigned char [bytes_to_read];
	read(new_sock, input_buffer, bytes_to_read);


	// output input_buffer to file
	ofstream outfile("paramsarrival", ios_base::binary);
	for(int i=0; i<bytes_to_read; i++)
		outfile << input_buffer[i];
	outfile.close();

	// Load stream into params
	seal::EncryptionParameters params;
	ifstream infile("paramsarrival", ios_base::binary);
	params.load(infile);

	// Initialize CRT Batcher and Evaluator
	seal::SEALContext context(params);
	seal::PolyCRTBuilder sCrtbuilder(context);
	seal::Evaluator evaluator(context);

	// Create evaluation and galois keys
	seal::KeyGenerator keygen(context);
	seal::GaloisKeys gal_keys;
	keygen.generate_galois_keys(30, gal_keys);
	seal::EvaluationKeys eval_keys;
	keygen.generate_evaluation_keys(30, eval_keys);	

	// Read and encode binary weights (most significant step to be done in advance)
	vector<vector<int64_t>> store0;
	vector<vector<int64_t>> store1;
	vector<seal::Plaintext> w0;
	vector<seal::Plaintext> w1;
	read_binaryWeightChunksShmeared(&store0, HIDDEN, INPUT+HIDDEN-1, "trained_Bweights_0");
	read_binaryWeightChunksShmeared(&store1, OUTPUT, HIDDEN+OUTPUT-1, "trained_Bweights_1");
	for(int i=0; i<INPUT+HIDDEN-1; i++)
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

	// ###########	PREPARATION PHASE OVER ############
	cout << "############   SERVER PREPARATION COMPLETE   ###########" << "\n" << endl;
	// LOOP BEGINS HERE


	// Receive ciphertext image
/*
	ofstream cipherout("cipherarrival", ios_base::binary | ios_base::trunc);
	unsigned char input_buf [1447];
	for(int i=0; i<68; i++)
	{
		read(new_sock, &input_buf, 1447);
		for(int ii=0; ii<1447; ii++)
		{
			cipherout << input_buf[ii];
		}
	}
	

*/
	unsigned char ready;
	read(new_sock, &ready, 1); 

	ifstream cipherin("cipherserial", ios_base::binary);
	seal::Ciphertext encImg;
	encImg.load(cipherin);
	cipherin.close();
	cout << "loaded ciphertext, sending back... " << endl;
	ofstream cipherout("cipherreturn", ios_base::binary | ios_base::trunc);
	encImg.save(cipherout);
	unsigned char done = 'd';
	write(new_sock, &done, 1);
	


//	evaluator.multiply(encImg, encImg);
//	cout << "SUCCESS multplying ciphertexts" << endl;



	// Generate image automorphisms
	// FC layer 1
	seal::Ciphertext sum;
	seal::Ciphertext image_autom(encImg);
	evaluator.rotate_rows(image_autom, 1-HIDDEN, gal_keys);
	evaluator.multiply_plain(image_autom, w0[0], sum);
	for(int i=1; i<INPUT+HIDDEN-1; i++)
	{
		int rotate_amount = i - (HIDDEN-1);
		seal::Ciphertext image_autom(encImg);
		evaluator.rotate_rows(image_autom, rotate_amount, gal_keys);
		evaluator.multiply_plain(image_autom, w0[i]);
		evaluator.add(sum, image_autom);
	}
	
	// Square activation
	evaluator.multiply(sum, sum);
	evaluator.relinearize(sum, eval_keys);


	// Return result back to client
	ofstream outresult("resultserial", ios_base::binary | ios_base::trunc);
	encImg.save(outresult);
	outresult.close();
	ifstream inresult("resultserial", ios_base::binary);
	unsigned char *res_buffer = new unsigned char [98396];
	char ccc;
	int cntt=0;
	while(inresult.get(ccc))
		res_buffer[cntt++] = ccc;
	if(write(new_sock, res_buffer, cntt)<0)
		cout << "Error writing output ciphertext to socket!" << endl;


	
	// FC layer 2
	// First, rotate previous activation HIDDEN times starting from (OUTPUT-1 right) to (HIDDEN-1 left). 
	// Then, multiply each of these ciphertexts (families) by their corresponding weights (family members)
	// and add them all together. Add all HIDDEN+OUTPUT-1 of these together. Only the first 10 elements of the
	// result matter. 
	seal::Ciphertext result;
	seal::Ciphertext sum_autom(sum);
	evaluator.rotate_rows(sum_autom, 1-OUTPUT, gal_keys);
	evaluator.multiply_plain(sum_autom, w1[0], result);
	for(int i=1; i<HIDDEN+OUTPUT-1; i++)
	{
		int rotate_amount = i - (OUTPUT-1);
		seal::Ciphertext sum_autom(sum);
		evaluator.rotate_rows(sum_autom, rotate_amount, gal_keys);
		evaluator.multiply_plain(sum_autom, w1[i]);
		evaluator.add(result, sum_autom);
	}

	





/*
	// Read ciphertext message
	int count=0;
	unsigned char ack = 'a';
	unsigned char ready = 'r';
	while(1)
	{
		write(new_sock, &ready, 1); // This is to separate byte_length message and ciphertext message	
		count++;
		int bytes;
		read(new_sock, &bytes, sizeof(int));
		cout << "bytes to allocate: " << bytes << endl;
	//	write(new_sock, &ack, 1); // This is to separate byte_length message and ciphertext message
		unsigned char *input_buf = new unsigned char [bytes];

		unsigned char * pointer_copy = input_buf;
		int bytes_copy=bytes;
		while(bytes_copy>0)
		{
			int num = read(new_sock, pointer_copy, bytes_copy);
			if (num<0) cout<<"Read error!";

			bytes_copy-=num;
			pointer_copy+=num;
		}
		
		ofstream cipherout("cipherarrival", ios_base::binary | ios_base::trunc);
		for(int i=0; i<bytes; i++)
			cipherout << input_buf[i];
		cipherout.close();
		ifstream cipherin("cipherarrival", ios_base::binary);
		seal::Ciphertext cipher;
		cipher.load(cipherin);
		cipherin.close();
		cout << "IMPORTED CIPHERTEXT " << count << endl;
		delete input_buf;

	}
*/
//	evaluator.multiply(test,test);




	while(1)
	{
		// Wait here for further connections
		socklen_t clilen = sizeof(cli_addrs);
		bzero((char*)&cli_addrs, clilen);
		int new_sock = accept(sock, (struct sockaddr*)&cli_addrs, &clilen);
		

		
	}
	





	close(new_sock);

	return 0;

}

void read_socket(int sock, unsigned char *buf)
{
	// Read message concerning number of bytes to read
	int bytes_to_read;
	read(sock, &bytes_to_read, sizeof(int));

	// Read actual message
	read(sock, buf, bytes_to_read);
	
	
}

