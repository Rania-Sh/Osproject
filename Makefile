# Variables
CC     = gcc
CFLAGS = -Wall -Wextra -std=c11
LIBS   = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Milestone 1 – Dijkstra only
milestone1:
	$(CC) $(CFLAGS) dijkstra.c -o dijkstra

# Milestones 2 & 3 – single traveler GUI
milestone2: milestone3
milestone3:
	$(CC) $(CFLAGS) main.c animation.c animationui.c dijkstra.c -o sim $(LIBS)

# Milestone 4 – multiple travelers, parent computes paths
milestone4:
	$(CC) $(CFLAGS) main.c animation.c animationui.c dijkstra.c -o sim $(LIBS)

# Milestone 5 – IPC via pipes, children compute their own paths
milestone5:
	$(CC) $(CFLAGS) main5.c animation.c animationui.c dijkstra.c -o sim $(LIBS)

# Clean up
clean:
	rm -f dijkstra sim *.o
