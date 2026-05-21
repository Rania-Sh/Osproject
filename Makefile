# Variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 # Standard raylib flags for Linux/MinGW

# Target for Milestone 1
milestone1:
	$(CC) $(CFLAGS) dijkstra.c -o dijkstra

# Target for Milestone 2 & 3
milestone2: milestone3
milestone3:
	$(CC) $(CFLAGS) main.c animation.c animationui.c -o sim $(LIBS)
milestone4:
	$(CC) $(CFLAGS) main.c animation.c animationui.c dijkstra.c -o sim $(LIBS)

# Clean up
clean:
	rm -f dijkstra sim *.o
