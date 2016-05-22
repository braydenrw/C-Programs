COPTS=-g -c -std=c99 -Wall -pedantic
all: mftpserve.o mftp.o
	gcc mftp.o -o mftp
	gcc mftpserve.o -o mftpserve
	make clean

mftpserve.o: mftpserve.c mftp.h
	gcc $(COPTS) mftpserve.c

mftp.o: mftp.c mftp.h
	gcc $(COPTS) mftp.c

clobber:
	rm -rf mftp mftpserve *.o *~

clean:
	rm -rf *.o *~