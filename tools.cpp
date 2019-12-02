#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <math.h>
#include "tools.h"

// Defines convenient functions for reading the MNIST database and weights/biases files

#define HIDDEN 200

using namespace std;

void read_MNIST_test (int NumberOfImages, int DataOfAnImage, unsigned char *buf, unsigned char *label) { //{{{
    ifstream f_image("./t10k-images-idx3-ubyte", ios::binary);
    ifstream f_label("./t10k-labels-idx1-ubyte", ios::binary);
    
    if (f_image.is_open()) {

	int trash;
        f_image.read((char*)&trash, sizeof(int));
        f_image.read((char*)&trash, sizeof(int));
        f_image.read((char*)&trash, sizeof(int));
        f_image.read((char*)&trash, sizeof(int));
 
        for (int i=0;i<NumberOfImages;++i) {
          for (int r=0;r<28;++r) {
            for (int c=0; c<28; ++c) {
              unsigned char temp=0;
              f_image.read((char*)&temp,sizeof(temp));
              buf[i*28*28+r*28+c] = (unsigned char)((double)temp/25.5);
            }
          }
        }
    } else {
      printf("MNIST image open error!\n");
    }
    
    if (f_label.is_open()) {

        int trash;

        f_label.read((char*)&trash, sizeof(int));
        f_label.read((char*)&trash, sizeof(int));
        
        for (int i=0; i<NumberOfImages; ++i) {
          unsigned char temp=0;
          f_label.read((char*)&temp, sizeof(temp));
          label[i] = temp;
        }
    } else {
      printf("MNIST label open error!\n");
    }
} //}}}

void read_MNIST_train (int NumberOfImages, int DataOfAnImage, unsigned char *buf, unsigned char *label) { //{{{
    ifstream f_image("./train-images-idx3-ubyte", ios::binary);
    ifstream f_label("./train-labels-idx1-ubyte", ios::binary);
    
    if (f_image.is_open()) {

	int trash;
        f_image.read((char*)&trash, sizeof(int));
        f_image.read((char*)&trash, sizeof(int));
        f_image.read((char*)&trash, sizeof(int));
        f_image.read((char*)&trash, sizeof(int));
 
        for (int i=0;i<NumberOfImages;++i) {
          for (int r=0;r<28;++r) {
            for (int c=0; c<28; ++c) {
              unsigned char temp=0;
              f_image.read((char*)&temp,sizeof(temp));
              buf[i*28*28+r*28+c] = (unsigned char)((double)temp/25.5);
            }
          }
        }
    } else {
      printf("MNIST image open error!\n");
    }
    
    if (f_label.is_open()) {

        int trash;

        f_label.read((char*)&trash, sizeof(int));
        f_label.read((char*)&trash, sizeof(int));
        
        for (int i=0; i<NumberOfImages; ++i) {
          unsigned char temp=0;
          f_label.read((char*)&temp, sizeof(temp));
          label[i] = temp;
        }
    } else {
      printf("MNIST label open error!\n");
    }
} //}}}

void read_MNIST_single(vector<int64_t>* img, int img_to_extract)
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
			(*img).emplace_back((int64_t)((double)temp/25.5));
		}
	}
	else
		printf("MNIST image open error!\n");
}

void read_MNIST_single(double *img, int img_to_extract)
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
			img[i] = (double)temp/25.5;
		}
	}
	else
		printf("MNIST image open error!\n");
}


void read_MNIST_single_largeCipher(vector<vector<int64_t>>* img, int img_to_extract)
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
			vector<int64_t> pix;
			for(int m=0; m<HIDDEN; m++)
				pix.emplace_back((int64_t)((double)temp/25.5));
			(*img).emplace_back(pix);
		}
	}
	else
		printf("MNIST image open error!\n");
}


void read_params(double *params, int size, string filename)
{
	ifstream infile(filename);

	for(int i=0; i<size; i++)
	{
		double wb;
		infile >> wb;
		params[i] = wb;
	}
	infile.close();
}

double read_binaryWeightChunks(vector<vector<int64_t>> *store, int chunk_size, int num_chunks, string filename)
{
	ifstream infile(filename);

	// Pre-fill store with vectors
	for(int i=0; i<num_chunks; i++)
	{
		vector<int64_t> temp(chunk_size, 0);
		(*store).emplace_back(temp);
	}
	double scalar;
	for(int c=0; c<chunk_size; c++)
	{
		for(int i=0; i<num_chunks; i++)
		{
			double w;
			infile >> w;
			if(c==0 && i==0)
				scalar = abs(w);
			((*store)[i])[c] = sign(w);
		}
	}
	infile.close();
	return scalar;
}

double read_binaryWeightChunksShmeared(vector<vector<int64_t>> *store, int chunk_size, int num_chunks, string filename)
{
	ifstream infile(filename);
	// Fill "store" with "num_chunks" empty vectors
	for(int i=0; i<num_chunks; i++)
	{
		vector<int64_t> temp(chunk_size, 0);
		(*store).emplace_back(temp);
	}
	double scalar;
	// Fill while shmearing
	for(int i=0; i<chunk_size; i++)
	{
		for(int c=0; c<num_chunks-chunk_size+1; c++)
		{
			double w; 
			infile >> w;
			if(c==0 && i==0)
				scalar = abs(w);
			//((*store)[c+chunk_size-i-1])[i] = sign(w);
			((*store)[c+chunk_size-1-i])[i] = sign(w);
		}
	}
	infile.close();
	return scalar;
}

int64_t sign(double x)
{
	if(x<0)
		return -1;
	else
		return 1;
}

void write_params(double *params, int size, string filename)
{
	ofstream outfile(filename);
	for(int i=0; i<size; i++)
	{
		outfile << params[i] << "\n";
	}
	outfile.close();
}

void display_image(double *img)
{
	cout << "##########     IMG OUTPUT     ###########\n\n";
	for(int i=0; i<784; i++)
	{
		if((int)img[i]==0)
			cout << "   ,";
		else if((int)img[i]<10)
			cout << (int)img[i] << "  ,";
		else if(img[i]<100)
			cout << (int)img[i] << " ,";
		else
			cout << (int)img[i] << ",";
		if(i%28==27)
			cout << endl;
	}
	cout << "\n\n\n";
}

void display_image(unsigned char *img)
{
	cout << "##########     IMG OUTPUT     ###########\n\n";
	for(int i=0; i<784; i++)
	{
		if((int)img[i]==0)
			cout << "   ,";
		else if((int)img[i]<10)
			cout << (int)img[i] << "  ,";
		else if(img[i]<100)
			cout << (int)img[i] << " ,";
		else
			cout << (int)img[i] << ",";
		if(i%28==27)
			cout << endl;
	}
	cout << "\n\n\n";
}

void binarize_deterministic(double *x, int size)
{
	double sum = 0;
	for(int i=0; i<size; i++)
		sum += abs(x[i]);
	for(int i=0; i<size; i++)
		if(x[i]<0)
			x[i] = -1*sum/(double)size;
		else
			x[i] = sum/(double)size;
}

void binarize_deterministic(double *source, double *dest, int size)
{
	double sum = 0;
	for(int i=0; i<size; i++)
		sum += abs(source[i]);
	for(int i=0; i<size; i++)
		if(source[i]<0)
			dest[i] = -1*sum/(double)size;
		else
			dest[i] = sum/(double)size;
}

void binarize_fake(double *source, double *dest, int size)
{
	for(int i=0; i<size; i++)
		dest[i] = source[i];
}

void binarize_stochastic(double *x, int size)
{
	// Calculate sum first
	double sum = 0;
	for(int i=0; i<size; i++)
		sum += abs(x[i]);

	// Binarize
	for(int i=0; i<size; i++)
	{
		double prob_of_1 = 0;
		if(x[i]>1)
			x[i] = sum/(double)size;
		else if(x[i]<-1)
			x[i] = -1*sum/(double)size;
		else
		{
			prob_of_1 = 0.5 + x[i]*0.5;
			x[i] = probability(prob_of_1)*sum/(double)size;
		}
	}
}

void binarize_stochastic(double *source, double *dest, int size)
{
	// Calculate sum first
	double sum = 0;
	for(int i=0; i<size; i++)
		sum += abs(source[i]);

	// Binarize
	for(int i=0; i<size; i++)
	{
		double prob_of_1 = 0;
		if(source[i]>1)
			dest[i] = sum/(double)size;
		else if(source[i]<-1)
			dest[i] = -1*sum/(double)size;
		else
		{
			prob_of_1 = 0.5 + source[i]*0.5;
			dest[i] = probability(prob_of_1)*sum/(double)size;
		}
	}
}

int probability(double prob_of_1)
{
	int resolution = 100000;
	int randnum = rand() % resolution;
	if(randnum <= resolution*prob_of_1)
		return 1;
	return -1;
}

void read_SingleImageFile(vector<vector<int64_t>> *store, int chunk_size, int num_chunks, string filename)
{
	ifstream infile(filename);
	for(int i=0; i<num_chunks; i++)
	{
		int64_t pixel;
		infile >> pixel;
		vector<int64_t> temp;
		for(int h=0; h<chunk_size; h++)
			temp.emplace_back(pixel);
		
		(*store).emplace_back(temp);
	}
	infile.close();
}

void read_SingleImageFile(vector<int64_t> *store, int size, string filename)
{
	ifstream infile(filename);
	for(int i=0; i<size; i++)
	{
		int64_t pixel;
		infile >> pixel;		
		(*store).emplace_back(pixel);
	}
	infile.close();
}

void putMNIST_in_Image_file(int img_number_to_extract, string filename)
{
	vector<int64_t> img;
	read_MNIST_single(&img, img_number_to_extract);
	ofstream outfile("./drawing/image");
	for(int i=0; i<784; i++)
	{
		outfile << img[i] << "\n";
	}
	outfile.close();
}

