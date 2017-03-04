CC      = gcc
CFLAGS  = -g
LDFLAGS = -DFS_STANDALONE -I../../nucleus/include

BIN = fatcopy fatdel fatdir fatformat fatmd fattype
OBJ = fatcopy.o fatdel.o fatdir.o fatformat.o fatmd.o fattype.o fat.o

all: $(OBJ)
	$(CC) $(CFLAGS) -o fatcopy fatcopy.o fat.o
	$(CC) $(CFLAGS) -o fatdel fatdel.o fat.o
	$(CC) $(CFLAGS) -o fatdir fatdir.o fat.o
	$(CC) $(CFLAGS) -o fatformat fatformat.o
	$(CC) $(CFLAGS) -o fatmd fatmd.o fat.o
	$(CC) $(CFLAGS) -o fattype fattype.o fat.o

fat.o:
	$(CC) $(CFLAGS) -c ../../nucleus/fs/fat.c $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(BIN) $(OBJ)