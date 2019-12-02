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

int main()
{
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

	// Send unsigned char data_buffer containing PARAMS
	cout << "count = " << count << endl;
	if(write(sock, serial_buff, count) < 0)
		cout << "Error sending message2 to server" << endl;

	// ###########	PREPARATION PHASE OVER ############
	// LOOP BEGINS HERE

//    while(1)
//    {

	// HANG HERE UNTIL USER DRAWS...


	// Obtain image from file (LOW MEMORY FLAVOR)
	vector<int64_t> img;
	putMNIST_in_Image_file(rand()%365, "./drawing/image");
	read_SingleImageFile(&img, INPUT, "./drawing/image");

	// FOR DISPLAY ONLY ##########################
	unsigned char imagen [INPUT];
	for(int i=0; i<INPUT; i++)
		imagen[i] = (unsigned char)img[i];
	display_image(imagen);
	// ###########################################

	// Encode and encrypt image
	cout << "ENCRYPTING IMAGE..." << "\n" << endl;
	clock_t encrypt_t = clock();
	seal::Ciphertext cipherImg;
	seal::Plaintext plainImg;
	crtbuilder.compose(img, plainImg);
	encryptor.encrypt(plainImg, cipherImg);
	encrypt_t = clock() - encrypt_t;
	cout << "Noise budget in fresh ciphertext: " << decryptor.invariant_noise_budget(cipherImg) << " bits" << endl;

	// Send ciphertext off to server
	unsigned char ready = 'r';
	write(sock, &ready, 1);
	ofstream cipherout("cipherserial", ios_base::binary | ios_base::trunc);
	cipherImg.save(cipherout);
	cipherout.close();
	unsigned char done;
	read(sock, &done, 1);
	ifstream cipherin("cipherreturn", ios_base::binary);
	seal::Ciphertext returned;
	cout << "test" << "\n" << endl;
	returned.load(cipherin);
	seal::Plaintext returnedPlain;
	decryptor.decrypt(returned, returnedPlain);
	vector<int64_t> returned_data;
	crtbuilder.decompose(returnedPlain, returned_data);
	for(int i=0; i<784; i++)
		cout << (int)returned_data[i] << ", ";
	

//	unsigned char cipher_buffer [1447];
//	ifstream cipherin("cipherserial", ios_base::binary);
//	for(int i=0; i<68; i++)
//	{
//		for(int ii=0; ii<1447; ii++)
//		{
//			cipherin >> cipher_buffer[ii];
//		}
//		if(write(sock, &cipher_buffer, 1447) < 0)
//			cout << "Error sending ciphertext " << "\n" << endl;
//	}




/*
	char cc;
	int cnt = 0;
	while(cipherin.get(cc))
		cipher_buffer[cnt++] = cc;
	cipherin.close();
	if(write(sock, cipher_buffer, 98396) < 0)
		cout << "Error sending ciphertext " << "\n" << endl;
*/
	
	// Wait for server response
	unsigned char *response = new unsigned char [98396];
	read(sock, response, 98396);
	ofstream respout("responsearrival", ios_base::binary | ios_base::trunc);
	for(int i=0; i<98396; i++)
		respout << response[i];
	respout.close();
	ifstream respin("responsearrival", ios_base::binary);
	seal::Ciphertext cipher_back;
	cipher_back.load(respin);
	respin.close();


	clock_t decrypt_t = clock();
	seal::Plaintext result_plain;
	vector<int64_t> output;
	decryptor.decrypt(cipher_back, result_plain);
	crtbuilder.decompose(result_plain, output);
	decrypt_t = clock() - decrypt_t;


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

	for(int i=0; i<784; i++)
		cout << output[i] << ", ";
	cout << "\n" << endl;

	cout << "Predicted digit: " << maximal_index << "\n" << endl;

	
	





/*
// Send ciphertexts off to server
int iter = 0;
while(iter<784)
{
	unsigned char ready;
	read(sock, &ready, 1);
	if(ready=='r') 
	{
		ofstream cipherout("cipherserial", ios_base::binary | ios_base::trunc);
		cipherImg[iter].save(cipherout);
		cipherout.close();
		unsigned char *cipher_buffer1 = new unsigned char [65535];
		ifstream cipherin("cipherserial", ios_base::binary);
		char cc;
		int cnt = 0;
		while(cipherin.get(cc))
			cipher_buffer1[cnt++] = cc;

		cout << iter << " Ciphertxt size: " << cnt << endl;
		if(write(sock, &cnt, sizeof(int)) < 0)
			cout << "Error sending ciphertext " << 0 << "\n" << endl;
	//	unsigned char ack;
	//	read(sock, &ack, 1);

		unsigned char * pointer_copy = cipher_buffer1;
		while(cnt>0)
		{
			int num = write(sock, pointer_copy, cnt);
			if (num<0)
				cout << "Error sending ciphertext " << 0 << "\n" << endl;

			pointer_copy+=num;
			cnt-=num;
		}

		cipherin.close();
		delete cipher_buffer1;
		iter++;
	}

}
*/
//    }





	close(sock);


	return 0;
}

