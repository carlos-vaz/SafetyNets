# REMOVE OLD CMAKE FILES
rm -r CMakeFiles; 
rm Makefile;
rm CMakeCache.txt;
rm cmake_install.cmake;
#rm encconv
#rm plainconv
#rm compare
#rm crtconv
#rm plainconv_intW
#rm encNN
#rm serial_test
#rm client_test
#rm server_test
#rm demo
rm client_demo
rm server_demo

# RE-MAKE FILES
cmake -DCMAKE_C_COMPILER=/usr/local/bin/gcc -DCMAKE_CXX_COMPILER=/usr/local/bin/g++ .;
make;

