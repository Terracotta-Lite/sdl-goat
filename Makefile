# build an executable named goat from main.c
all: goat.c
	gcc -g -Wall -lSDL2 -lSDL2_mixer -lSDL2_image -o goat goat.c

clean: 
	$(RM) goat
