cmake_minimum_required(VERSION 3.12)
project(Trabalho_1 C)

set(CMAKE_C_STANDARD 11)

add_executable(Trabalho_1
        process_simulator.c)

target_link_libraries(Trabalho_1 PRIVATE -lsodium -lpthread)