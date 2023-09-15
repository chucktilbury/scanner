CC		=	gcc
TARGET	=	scanner
OBJS	=	scanner.o \
			fileio.o \
			memory.o \
			tokens.o \
			strs.o
DEBUG	=	-g
COPTS	=	-Wall -Wextra -DUSE_GC
LOPTS	=
LIBS	=	-lgc

%.o:%.c
	$(CC) $(DEBUG) $(COPTS) -c -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(DEBUG) $(LOPTS) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(TARGET) $(OBJS)
