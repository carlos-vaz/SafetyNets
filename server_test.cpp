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


	// Wait for 2 connections, accept when they arrive, and store messages
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
	seal::SEALContext context(params);

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
