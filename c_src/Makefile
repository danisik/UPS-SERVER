CC = gcc
BIN = server
OBJ = client.o server.o

all: $(BIN) clean

$(BIN): $(OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@
  
clean:	
	rm -f *.o
