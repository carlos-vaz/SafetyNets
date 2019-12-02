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
 
#define countof(ARRAY) (sizeof(ARRAY) / sizeof*(ARRAY))

using namespace std;

int reverse_int (int i) { //{{{
    unsigned char ch1, ch2, ch3, ch4;
    
    ch1=i&255;
    ch2=(i>>8)&255;
    ch3=(i>>16)&255;
    ch4=(i>>24)&255;
    
    return((int)ch1<<24)+((int)ch2<<16)+((int)ch3<<8)+ch4;
} //}}}

void read_MNIST (int NumberOfImages, int DataOfAnImage, unsigned char *buf, unsigned char *label) { //{{{
    ifstream f_image("./t10k-images-idx3-ubyte", ios::binary);
    ifstream f_label("./t10k-labels-idx1-ubyte", ios::binary);
    
    if (f_image.is_open()) {
        int magic_number=0;
        int number_of_images=0;
        int n_rows=0;
        int n_cols=0;

        f_image.read((char*)&magic_number, sizeof(magic_number));
        magic_number= reverse_int(magic_number);
        f_image.read((char*)&number_of_images, sizeof(number_of_images));
        number_of_images= NumberOfImages;
        f_image.read((char*)&n_rows, sizeof(n_rows));
        n_rows= reverse_int(n_rows);
        f_image.read((char*)&n_cols, sizeof(n_cols));
        n_cols= reverse_int(n_cols);
        
        for (int i=0;i<number_of_images;++i) {
          for (int r=0;r<n_rows;++r) {
            for (int c=0; c<n_cols; ++c) {
              unsigned char temp=0;
              f_image.read((char*)&temp,sizeof(temp));
              buf[i*n_rows*n_cols+r*n_cols+c] = temp;
            }
          }
        }
    } else {
      printf("MNIST image open error!\n");
    }
    
    if (f_label.is_open()) {
        int magic_number=0;
        int number_of_images=0;
        int n_rows=0;
        int n_cols=0;

        f_label.read((char*)&magic_number, sizeof(magic_number));
        magic_number= reverse_int(magic_number);
        f_label.read((char*)&number_of_images, sizeof(number_of_images));
        number_of_images= NumberOfImages;
        
        for (int i=0; i<number_of_images; ++i) {
          unsigned char temp=0;
          f_label.read((char*)&temp, sizeof(temp));
          label[i] = temp;
        }
    } else {
      printf("MNIST label open error!\n");
    }
} //}}}

// conv1 		 1	5     28       img    weights	conv sum
// conv2		 5     50     14
int conv (int layer, int D, int N, int W, float *a, float *w, float *sum) { //{{{
  for (int n=0; n<N; n++)
  for (int z=0; z<D; z++)
  for (int y=0; y<W; y++)
  for (int x=0; x<W; x++) {
    for (int yy=0; yy<5; yy++)
    for (int xx=0; xx<5; xx++) {
      if (((y ==   0) && ((yy == 0) || (yy == 1))) ||  
          ((y ==   1) && ((yy == 0))) ||  
          ((y == W-2) && ((yy == 4))) ||  
          ((y == W-1) && ((yy == 3) || (yy == 4))) ||  
          ((x ==   0) && ((xx == 0) || (xx == 1))) ||  
          ((x ==   1) && ((xx == 0))) ||  
          ((x == W-2) && ((xx == 4))) ||
          ((x == W-1) && ((xx == 3) || (xx == 4)))) {
        sum[n*W*W+y*W+x] += 0; // Boundary
      } else {
        sum[n*W*W+y*W+x] += (w[yy*D*5*N+xx*D*N+z*N+n] * a[z*W*W+(y-2+yy)*W+(x-2+xx)]);
      }   
    }   
  }

  return 0;
} //}}}

int square_act (int layer, int N, int W, float *i_a, float *o_a) { //{{{
  for (int n=0; n<N; n++)
  for (int y=0; y<W; y++)
  for (int x=0; x<W; x++) {
    o_a[n*W*W+y*W+x] = i_a[n*W*W+y*W+x] * i_a[n*W*W+y*W+x];
  }

  return 0;
} //}}}

int avg_pool (int layer, int N, int W, float *i_a, float *o_a) { //{{{
  for (int n=0; n<N; n++)
  for (int y=0; y<W; y+=2)
  for (int x=0; x<W; x+=2) {
    o_a[n*W/2*W/2+y/2*W/2+x/2] = (i_a[n*W*W+(y  )*W+x] 
                                + i_a[n*W*W+(y  )*W+(x+1)] 
                                + i_a[n*W*W+(y+1)*W+x] 
                                + i_a[n*W*W+(y+1)*W+(x+1)]) / 4;
  }

  return 0;
} //}}}

int fc (int layer, int W, int H, float *a, float *w, float *sum) { //{{{
  for (int j=0; j<W; j++)
  for (int i=0; i<H; i++) {
    sum[j] += (a[i] * w[i*W+j]);
  }

  return 0;
} //}}}

int fc_square_act (int layer, int W, float *i_a, float *o_a) { //{{{
  for (int x=0; x<W; x++) {
    o_a[x] = i_a[x] * i_a[x];
  }

  return 0;
} //}}}

int reshape (int layer, int N, int W, float *i_a, float *o_a) { //{{{
  for (int n=0; n<N; n++)
  for (int yy=0; yy<W; yy++)
  for (int xx=0; xx<W; xx++) {
    o_a[yy*N*W+xx*N+n] = i_a[n*W*W+yy*W+xx];
  }
  
  return 0;
} //}}}

int fc_sigmoid_act (int W, float *i_a, float *o_a) { //{{{
  for (int x=0; x<W; x++) {
    o_a[x] = 1 / (1 + exp(-i_a[x]));
  }

  return 0;
} //}}}

char classify (int W, float *o_a) { //{{{
  float max = 0;
  char idx = 0;

  for (int i=0; i<W; i++) {
    if (max < o_a[i]) {
      max = o_a[i];
      idx = (char)i;
    }
  }

  return idx;
} //}}}

char nn (float *i_img, float *w_conv1, float *w_conv2, float *w_fc1, float *w_fc2) { //{{{
  char ret;

  // Arrays for convolution sums.
  float s_conv1    [28*28*5];
  float s_conv2    [14*14*50];
  float s_fc1      [100];
  float s_fc2      [10];

  // Arrays for activations.
  float a_conv1    [28*28*5];
  float a_conv2    [14*14*50];
  float a_fc1      [100];
  float a_fc2      [10];

  // Arrays for activations after pooling and reshaping.
  float a_conv1_PL [14*14*5];
  float a_conv2_PL [7*7*50];
  float a_conv2_RS [7*7*50];
  
  // Memory reset.
  memset(s_conv1, 0, sizeof(s_conv1));
  memset(s_conv2, 0, sizeof(s_conv2));
  memset(s_fc1, 0, sizeof(s_fc1));
  memset(s_fc2, 0, sizeof(s_fc2));
  memset(a_conv1, 0, sizeof(a_conv1));
  memset(a_conv2, 0, sizeof(a_conv2));
  memset(a_fc1, 0, sizeof(a_fc1));
  memset(a_fc2, 0, sizeof(a_fc2));
  memset(a_conv1_PL, 0, sizeof(a_conv1_PL));
  memset(a_conv2_PL, 0, sizeof(a_conv2_PL));
  memset(a_conv2_RS, 0, sizeof(a_conv2_RS));
  
  // Conv1.
  conv(1, 1, 5, 28, i_img, w_conv1, s_conv1);
  // Square activation.
  square_act(1, 5, 28, s_conv1, a_conv1);
  // AVG pool.
  avg_pool(1, 5, 28, a_conv1, a_conv1_PL);
  
  // Conv2.
  conv(2, 5, 50, 14, a_conv1_PL, w_conv2, s_conv2);
  // AVG pool.
  avg_pool(2, 50, 14, s_conv2, a_conv2_PL);
  // Reshape.
  reshape(2, 50, 7, a_conv2_PL, a_conv2_RS);
  
  // FC1 (W = 100, H = 7x7x50).
  fc(3, 100, 2450, a_conv2_RS, w_fc1, s_fc1);
  // Square activation.
  fc_square_act(3, 100, s_fc1, a_fc1);
  
  // FC2 (W = 10, H = 100).
  fc(4, 10, 100, a_fc1, w_fc2, s_fc2);
  // Sigmoid activation.
  fc_sigmoid_act (10, s_fc2, a_fc2);
  
  ret = classify(10, a_fc2);

  return ret;
} //}}}

int main() { //{{{
  int no_img = 10000;
  char pred;
  unsigned int correct = 0;
  unsigned char *buf   = (unsigned char *)malloc(28*28*no_img*sizeof(unsigned char));
  unsigned char *label = (unsigned char *)malloc(no_img*sizeof(unsigned char));
  float *buf_f = (float *)malloc(28*28*sizeof(float));
  
  // Read MNIST images and labels.
  read_MNIST(no_img, 784, buf, label);
 
  // Read weights.
  float w_conv1    [5*5*1*5];
  float w_conv2    [5*5*5*50];
  float w_fc1      [7*7*50*100];
  float w_fc2      [100*10];

  FILE *f_w_conv1 = fopen("./W_conv1.ckpt.data-00000-of-00001", "rb");
  FILE *f_w_conv2 = fopen("./W_conv2.ckpt.data-00000-of-00001", "rb");
  FILE *f_w_fc1   = fopen("./W_fc1.ckpt.data-00000-of-00001", "rb");
  FILE *f_w_fc2   = fopen("./W_fc2.ckpt.data-00000-of-00001", "rb");
  assert(f_w_conv1);
  assert(f_w_conv2);
  assert(f_w_fc1);
  assert(f_w_fc2);

  size_t n_w_conv1 = fread(w_conv1, sizeof(float), countof(w_conv1), f_w_conv1);
  size_t n_w_conv2 = fread(w_conv2, sizeof(float), countof(w_conv2), f_w_conv2);
  size_t n_w_fc1   = fread(w_fc1, sizeof(float), countof(w_fc1), f_w_fc1);
  size_t n_w_fc2   = fread(w_fc2, sizeof(float), countof(w_fc2), f_w_fc2);
  assert(n_w_conv1 == countof(w_conv1));
  assert(n_w_conv2 == countof(w_conv2));
  assert(n_w_fc1 == countof(w_fc1));
  assert(n_w_fc2 == countof(w_fc2));

  // ML inference.
  for (int n=0; n<no_img; n++) {
    for (int y=0; y<28; y++)
    for (int x=0; x<28; x++) {
      buf_f[y*28+x] = ((float)buf[n*28*28+y*28+x])/255;
    }

    pred = nn(buf_f, w_conv1, w_conv2, w_fc1, w_fc2);

    if (pred == label[n]) correct++;
    if (n%100 == 99) printf("n: %5d / inference accuracy: %f\n", n, ((float)correct)/(n+1));
  }

  // Buffer free.
  free(buf);
  free(buf_f);
  free(label);

  return 0;
} //}}}
