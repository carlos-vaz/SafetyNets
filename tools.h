#include <String>
#include <vector>

#ifndef TOOLS_H
#define TOOLS_H

void read_params(double*, int, std::string);
void write_params(double*, int, std::string);
double read_binaryWeightChunks(std::vector<std::vector<int64_t>>*, int, int, std::string);
double read_binaryWeightChunksShmeared(std::vector<std::vector<int64_t>> *store, int chunk_size, int num_chunks, std::string filename);
int64_t sign(double);
void read_MNIST_test (int, int, unsigned char*, unsigned char*);
void read_MNIST_train (int, int, unsigned char*, unsigned char*);
void read_MNIST_single(std::vector<int64_t>*, int);
void read_MNIST_single(double *img, int img_to_extract);
void read_MNIST_single_largeCipher(std::vector<std::vector<int64_t>>*, int);
void display_image(double*);
void display_image(unsigned char*);
void binarize_deterministic(double*, int);
void binarize_deterministic(double*, double*, int);
void binarize_fake(double*, double*, int);
void binarize_stochastic(double*, int);
void binarize_stochastic(double*, double*, int);
int probability(double);
void read_SingleImageFile(std::vector<std::vector<int64_t>> *store, int chunk_size, int num_chunks, std::string filename);
void read_SingleImageFile(std::vector<int64_t> *store, int size, std::string filename);
void putMNIST_in_Image_file(int img_number_to_extract, std::string filename);

#endif
