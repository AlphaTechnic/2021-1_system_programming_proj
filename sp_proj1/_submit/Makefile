CC = gcc
CFLAGS = -W -Wall
TARGET = 20121277.out
OBJECTS = 20121277.o utils.o shell_commands.o memory_commands.o opcode_table_commands.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

clean :
	rm *.o
	rm *.out
