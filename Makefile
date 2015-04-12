CC = clang
EXECNAME=farm
CFLAGS= -I include/ -Wall -Wextra -Wpedantic -Ofast

all: main.o md5.o comm.o
	$(CC) -o  $(EXECNAME) main.o comm.o md5.o -lpthread

clean:
	rm -f *.o
	rm -f $(EXECNAME)

