cmake_minimum_required(VERSION 3.10)

find_package(Threads)

# Import SEAL
find_package(SEAL 2.3.1 EXACT REQUIRED HINTS “~/Documents/SEAL_2.3.1/SEAL/cmake/”)

# Compile tools as well
set(SOURCES encNN.cpp tools.cpp)
set(SOURCES2 demo.cpp tools.cpp)
set(SOURCES3 client_demo.cpp tools.cpp)
set(SOURCES4 server_demo.cpp tools.cpp)

#add_executable(encconv encconv.cpp)
#add_executable(plainconv plainconv.cpp)
#add_executable(compare compare.cpp)
#add_executable(crtconv crtconv.cpp)
#add_executable(plainconv_intW plainconv_intW.cpp)
#add_executable(encNN ${SOURCES})
#add_executable(serial_test serial_test.cpp)
#add_executable(client_test client_test.cpp)
#add_executable(server_test server_test.cpp)
#add_executable(demo ${SOURCES2})
add_executable(client_demo ${SOURCES3})
add_executable(server_demo ${SOURCES4})


# Link SEAL
#target_link_libraries(encconv SEAL::seal)
#target_link_libraries(crtconv SEAL::seal)
#target_link_libraries(encNN SEAL::seal)	
#target_link_libraries(serial_test SEAL::seal)
#target_link_libraries(client_test SEAL::seal)
#target_link_libraries(server_test SEAL::seal)
#target_link_libraries(demo SEAL::seal)
target_link_libraries(client_demo SEAL::seal)
target_link_libraries(server_demo SEAL::seal)
