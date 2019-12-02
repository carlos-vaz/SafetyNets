#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ostream>
#include <iostream>
#include <unistd.h>

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
	
	// Enable listening for incoming connections (accept max 2 connections)
	if(listen(sock, 2)<0)
		std::cout << "Error listening on port 8081" << std::endl;

	// Wait for 2 connections, accept when they arrive, and store messages
	socklen_t clilen = sizeof(cli_addrs);
	float buffer_plain [28*28*5];
	float buffer_encr [28*28*5];
	float *buffer_addrs[]  = {&buffer_plain[0], &buffer_encr[0]};
	for(int arrival=0; arrival<2; arrival++)
	{
		bzero((char*)&cli_addrs, clilen);
		int new_sock = accept(sock, (struct sockaddr*)&cli_addrs, &clilen);

		// Read message from client, store in appropriate buffer
		read(new_sock, buffer_addrs[arrival], sizeof(buffer_plain));

		std::cout << "Client " << arrival+1 << " has connected" << std::endl;
		close(new_sock);

	}


	// Now that we have both convolution outputs, element-wise subtract them to compare
	std::cout << "########     DIFFERENCES     ########" << std::endl;
	float diff [28*28*5];
	memset(&diff, 0, sizeof(diff));
	float maxval = 0;
	float minval = 0;
	for(int i=0; i<28*28*5; i++)
	{
		diff[i] = buffer_plain[i] - buffer_encr[i];
		std::cout << diff[i] << " ,";
		if(diff[i]>maxval)
			maxval = diff[i];
		if(diff[i]<minval)
			minval = diff[i];
	}
	std::cout << "\n" << std::endl;

	std::cout << "MAXIMUM VALUE IN DIFF: " << maxval << std::endl;
	std::cout << "MINIMUM VALUE IN DIFF: " << minval << std::endl;
	return 0;

}
