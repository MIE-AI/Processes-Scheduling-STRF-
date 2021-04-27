CC=gcc
SRCS=allocate.o cpu_task.o process.o utility.o
TARGET=allocate
CFLAGS=-Wall

allocate: $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) -lm

allocate.o: allocate.c utility.h process.h cpu_task.h
	$(CC) $(CFLAGS) -c allocate.c

utility.o: utility.c utility.h
	$(CC) $(CFLAGS) -c utility.c
 
process.o: process.c process.h utility.h
	$(CC) $(CFLAGS) -c process.c
 
cpu_task.o: cpu_task.c cpu_task.h utility.h
	$(CC) $(CFLAGS) -c cpu_task.c

.PHONY: clean

clean: 
	rm -f *.o
	rm -f allocate