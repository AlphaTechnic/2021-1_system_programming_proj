cmake_minimum_required(VERSION 3.17)
project(sp_proj5 C)

set(CMAKE_C_STANDARD 11)

add_executable(sp_proj5 csapp.c echo.c multiclient.c stockserver.c)