cmake_minimum_required(VERSION 3.17)
project(sp_proj2_3 C)

set(CMAKE_C_STANDARD 99)

add_executable(sp_proj2_3 20121277.c assembler_commands.c memory_commands.c opcode_table_commands.c
        shell_commands.c utils.c)