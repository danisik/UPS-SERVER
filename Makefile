CC = gcc
BIN = server.exe
OBJ = client.o conditions.o game.o server.o

all: $(BIN) move clean

$(BIN): $(OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@

move: 
	mv server.exe ../

clean:	
	rm -f *.o
