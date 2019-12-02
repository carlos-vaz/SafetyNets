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

// As a simple example, I want to try to send params through a socket to the process server_test.cpp

	// ########	CLIENT	    ##########
	seal::EncryptionParameters params;
	params.set_poly_modulus("1x^2048 + 1");
	params.set_plain_modulus(40960001);
	vector<seal::SmallModulus> small_moduli = {seal::small_mods_60bit(0), seal::small_mods_60bit(1),
	seal::small_mods_60bit(2)};
	params.set_coeff_modulus(small_moduli);
	seal::SEALContext context(params);
	
	// Key Generation
	seal::KeyGenerator keygen(context);
	auto public_key = keygen.public_key();
	auto secret_key = keygen.secret_key();

	// CRT encoder, encryptor, & decryptor init
	seal::PolyCRTBuilder crtbuilder(context); 
	seal::Encryptor encryptor(context, public_key);
	seal::Decryptor decryptor(context, secret_key);

// This is the part I'm not sure about. I need to somehow send the params output stream 'outfile' in binary format through the socket

	// Save params to output stream
	ofstream outfile("paramsserial", ios_base::binary);
	params.save(outfile);
	outfile.close();

	ifstream infile("paramsserial", ios_base::binary);
	unsigned char *serial_buff = new unsigned char [100000];
	char c;
	int count = 0;
	while(infile.get(c))
	{
		serial_buff[count++] = c;
	}
	
	// Create socket to send params and ciphertext
	struct sockaddr_in server_addrs;
	bzero((char*)&server_addrs, sizeof(server_addrs));
	server_addrs.sin_family = AF_INET;
	server_addrs.sin_port = htons(8081);
	if(inet_pton(AF_INET, "127.0.0.1", &server_addrs.sin_addr)<=0)
		cout << "Error converting localhost to binary IP address" << endl;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(sock, (struct sockaddr*)&server_addrs, sizeof(server_addrs))<0)
		cout << "Error connecting to process 'compare'" << endl;

	// Send int telling server how many bytes to read
	int bytes_to_read = count;
	if(write(sock, &bytes_to_read, sizeof(int)) < 0)
		cout << "Error sending message1 to server" << endl;


	// Send unsigned char data_buffer
	cout << "count = " << count << endl;
	if(write(sock, serial_buff, count) < 0)
		cout << "Error sending message2 to server" << endl;

	close(sock);


	return 0;
}
