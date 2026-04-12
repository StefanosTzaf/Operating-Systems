CC = gcc

CFLAGS = -Wall -I ./include
OBJS = List.o Graph.o miris.o Map.o
EXEC = miris

$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(EXEC)
