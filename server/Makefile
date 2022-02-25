#Make file to compile aesdsocket

CROSS_COMPILE=

CC=$(CROSS_COMPILE)gcc

all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) -Wall -Werror -g aesdsocket.c -lpthread -o aesdsocket

clean:
	rm -rf aesdsocket
