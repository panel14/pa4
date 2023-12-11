CC=clang
CFLAGS=-std=c99 -Wall -pedantic
SOURCES=pa4.c bank_robbery.c ipc.c log.c process.c lamport_clock.c lamport_exclude.c queue.c
OBJECTS=$(SOURCES: .c=.o)
EXECUTABLE=pa4
EXEARGS=-p 3 --mutexl

all: rebuild run

rebuild: clean build

build: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ -L/home/locci/projects/pa4/lib -lruntime

clean:
	rm -rf *.o *.log \
	rm pa4

run:
	./$(EXECUTABLE) $(EXEARGS)