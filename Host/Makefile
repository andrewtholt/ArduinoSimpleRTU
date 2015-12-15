
CC=gcc
CFLAGS=-g

BINS=client sender listener sizes
LIBS=-ldl -lrt -pthread 

all:	$(BINS)

client.o:	client.c
	$(CC) -c $(CFLAGS) $? -o $@

sizes.o:	sizes.c
	$(CC) -c $(CFLAGS) $? -o $@

client:	client.o serialComms.o serialComms.h
	$(CC) $(CFLAGS) client.o serialComms.o -o $@

sender:	sender.o serialComms.o serialComms.h
	$(CC) $(CFLAGS) sender.o serialComms.o $(LIBS) -o $@

listener:	listener.o serialComms.o serialComms.h
	$(CC) $(CFLAGS) listener.o serialComms.o $(LIBS) -o $@

serialComms.o:	serialComms.c
	$(CC) -c $(CLAGS) $? -o $@

clean:
	rm -f $(BINS) *.o cscope.out

backup:	clean
	./backup.sh


