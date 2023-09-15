CC		=	gcc
TARGET	=	genscan
OBJS	=	scan.o \
			fileio.o \
			mem.o \
			tok.o \
			strs.o
INCS	=	-I.
DEBUG	=	-g
WARNS	=	-Wall -Wextra -Wpedantic
CFGS	=	-DUSE_GC
LOPTS	=
LIBS	=	-lgc

%.o:%.c
	$(CC) $(INCS) $(DEBUG) $(CFGS) $(WARNS) -c -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(DEBUG) $(LOPTS) -o $@ $(OBJS) $(LIBS)

clean:
	-rm -f $(TARGET) $(OBJS)
