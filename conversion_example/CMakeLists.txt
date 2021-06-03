cmake_minimum_required(VERSION 3.17)
project(conversion_example C)

set(CMAKE_C_STANDARD 11)

add_executable(conversion_example main.c csapp.c)