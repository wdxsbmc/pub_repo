#CC?=arm-linux-gcc
CC=gcc
LD=$(CC)
LIB = $(PWD)
INCLUDE = $(PWD)

CFLAGS = -Wall -c -lm -L$(LIB) -I$(INCLUDE) 
CFLAGS += -I$(PWD)
LDFLAGS = -L$(LIB) -lm -I$(INCLUDE) -ljpeg

SRCS = $(wildcard *.c source/*.c)
OBJS = $(patsubst %c, %o, $(SRCS))
TARGET=convertbmp2bin
 
.PHONY: all clean
all: $(TARGET) 
$(TARGET): $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) 
%o: %c
	$(CC) -o $@ $< $(CFLAGS)
clean:
	rm -f *.o *.cls *.hls $(TARGET)
