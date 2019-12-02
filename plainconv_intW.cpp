#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <limits>
#include <vector>
#include <string>
#include <chrono>
#include <assert.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

using namespace std;

#define countof(ARRAY) (sizeof(ARRAY) / sizeof*(ARRAY))


int reverse_int (int i) 
{
    unsigned char ch1, ch2, ch3, ch4;
    
    ch1=i&255;
    ch2=(i>>8)&255;
    ch3=(i>>16)&255;
    ch4=(i>>24)&255;
    
    return((int)ch1<<24)+((int)ch2<<16)+((int)ch3<<8)+ch4;
}


void read_weights(int* weights, int weights_count)
{
	float *weights_float = new float [weights_count];
	FILE *f = fopen("./W_conv1.ckpt.data-00000-of-00001", "rb");
	fread(weights_float, sizeof(float), weights_count, f);
	for(int i=0; i<weights_count; i++)
	{
		weights[i] = (int)(weights_float[i]*1000);
//		if(weights[i]<0)
//			weights[i] *= -1;
	}
	delete weights_float;
	cout << "##########     WEIGHTS     ###########\n" << endl;
	for(int c=0; c<5; c++)
	{
		int cycle = 0;
		for(int i=0; i<weights_count; i++)
		{
			if(i%5==c)
			{
				cout << weights[i] << ", ";
				cycle++;
				if(cycle%5==0)
					cout << "\n";
			}
		}
		cout << "\n\n" << endl;
	}
	fclose(f);
}

void read_MNIST(int *img, int img_to_extract)
{
	ifstream mnist("./t10k-images-idx3-ubyte", ios::binary);
	if (mnist.is_open()) 
	{
		int magic_number=0;
		int number_of_images=0;
		int n_rows=0;
		int n_cols=0;
	
		mnist.read((char*)&magic_number, sizeof(magic_number));
		mnist.read((char*)&number_of_images, sizeof(number_of_images));
		number_of_images = reverse_int(number_of_images);
		mnist.read((char*)&n_rows, sizeof(n_rows));
		n_rows = reverse_int(n_rows);
		mnist.read((char*)&n_cols, sizeof(n_cols));
		n_cols = reverse_int(n_cols);
	
		cout << "number of images: " << number_of_images << endl;
		cout << "number of rows: " << n_rows << endl;
		cout << "number of columns: " << n_cols << endl;
	
		for(int i=0; i<img_to_extract*784; i++)
		{
			unsigned char temp=0;
			mnist.read((char *)&temp, sizeof(temp));
		}

		for (int r=0;r<n_rows;++r) 
		{
			for (int c=0; c<n_cols; ++c) 
			{
				unsigned char temp=0;
				mnist.read((char *)&temp,sizeof(temp));
				img[r*n_cols+c] = (int)temp;
			}
		}
	}
	else 
	{
			printf("MNIST image open error!\n");
	}
	
	for (int i=0; i<784; i++)
	{
		if ((int)img[i]>250)
			cout << "::";
		else if ((int)img[i]>50)
			cout << " .";
		else 
			cout << "  ";
		if(i%28==27)
		{
			cout << endl;
		}
	}
	cout << "\n" << endl;
}


// conv1      1	     5     28       img    weights	conv sum
// conv2      5     50     14
int conv (int D, int N, int W, int *a, int *w, int *sum)
{
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
				sum[n*W*W+y*W+x] += 0; // Boundary
			} 
			else
			{
				sum[n*W*W+y*W+x] += (w[yy*D*5*N+xx*D*N+z*N+n] * a[z*W*W+(y-2+yy)*W+(x-2+xx)]);
			}
		}
	return 0;
}

void square_activ (int *in, int size)
{
	for(int i=0; i<size; i++)
		in[i] *= in[i];
}

void display_conv(int *sum, int sum_count)
{
	cout << "##########     CONV OUTPUT     ###########\n\n";
	for(int i=0; i<sum_count; i++)
	{
		if(i%784==0)
			cout << "\n\n";
		if(i%28==0)
			cout << "\n";
		int s = sum[i];
		if(s<10 && s>=0)
			cout << s << "   ,";
		else if(s<100 && s>-10)
			cout << s  << "  ,";
		else if(s>-100)
			cout << s << " ,";
		else
			cout << s << ",";
	}
	cout << "\n\n\n";
}

void display_img(int *img)
{
	cout << "##########     IMG OUTPUT     ###########\n\n";
	for(int i=0; i<784; i++)
	{
		if(img[i]<10)
			cout << img[i] << "  ,";
		else if(img[i]<100)
			cout << img[i] << " ,";
		else
			cout << img[i] << ",";
		if(i%28==27)
			cout << endl;
	}
	cout << "\n\n\n";
}

int main()
{
	// Create buffer for first image in MNIST test datase, for weights, & for convolution output images
	int *img = (int *)malloc(28*28*sizeof(int));
	int weights [5*5*1*5];
	int conv_out [28*28*5];
	memset(img, 0, 784);
	memset(weights, 0, sizeof(weights));
	memset(conv_out, 0, sizeof(conv_out));

	// Read image and weights
	read_MNIST(img, 17);
	read_weights(weights, countof(weights));
//	for(int i=0; i<784; i++)
//		img[i] = 1;
	display_img(img);

	// Apply first convolution layer (timed)
	clock_t t = clock();
	conv(1, 5, 28, img, weights, conv_out);

	// Apply square activation function
	square_activ(&conv_out[0], 28*28*5);

	t = clock() - t;

	// Display convolution results
	display_conv(conv_out, countof(conv_out));
	cout << "number of ticks for conv1 layer: " << t << " or " << (float)t/CLOCKS_PER_SEC << " seconds" << endl;

	// Send convolution results to process 'compare'
	struct sockaddr_in server_addrs;
	bzero((char*)&server_addrs, sizeof(server_addrs));
	server_addrs.sin_family = AF_INET;
	server_addrs.sin_port = htons(8081);
	if(inet_pton(AF_INET, "127.0.0.1", &server_addrs.sin_addr)<=0)
		cout << "Error converting localhost to binary IP address" << endl;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(sock, (struct sockaddr*)&server_addrs, sizeof(server_addrs))<0)
		cout << "Error connecting to process 'compare'" << endl;
	if(write(sock, &conv_out, sizeof(conv_out)) < 0)
		cout << "Error sending message to server" << endl;

	cout << "sizeof(conv_out): " << sizeof(conv_out) << endl;

	return 0;
}

