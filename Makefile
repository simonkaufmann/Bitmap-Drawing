SRC=list.c linked_list.c bitmap.c parse.c draw.c
OBJS=$(SRC:%.c=%.o)
OUTPUT=bitmap
CC=gcc
CFLAGS=-std=c99
CLFLAGS=-lm

all: $(OUTPUT)

$(OUTPUT): main.c main.h $(OBJS)
	$(CC) $(CFLGAS) -o $(OUTPUT) main.c $(OBJS) $(CLFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< $(CLFLAGS)

run: all
	./bitmap input.txt output.bmp 640 480

clean:
	rm -r -f $(OUTPUT)
	rm -r -f $(OBJS)
