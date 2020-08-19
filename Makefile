tetris: tetris.c
	gcc -o tetris tetris.c -lSDL2 -lSDL2main
run: tetris
	./tetris
