# SafetyNets
Neural networks on encrypted data using Microsoft SEAL

## Overview
We classify 28x28 images of handwritten digits from MNIST that have been encrypted with a scheme that takes advantage of the Ring Learning With Errors (LWE or RLWE) assumption. 


## `crtconv.cpp`
Performs a 5x5x5 convolution layer on an encrypted input image. We encrypt the whole image into a single ciphertext polynomial (degree 4096) by using SEAL's Chinese Remainder Theorem (CRT) batching functionality. We speed up the process by rotating the image 25 times (the size of each filter) 
using SEAL's Galois Automorphisms functions and pixel-wise multiplying each rotated image by the corresponding weights and summing them. This has the effect of everywhere moving (via the rotations) the 25 necessary pixels to converge at a single point, 
each with the appropriate weights, and then summing them. 

## Links
(Fully Homomorphic Encryption from LWE (no rings))[https://eprint.iacr.org/2011/344.pdf]
(Microsoft SEAL paper)[https://www.microsoft.com/en-us/research/wp-content/uploads/2017/06/sealmanual_v2.2.pdf]

