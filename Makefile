all: minesweeper

minesweeper: main.c
	clang -O3 -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL lib/libraylib.a -I include main.c -o minesweeper
clean:
	rm minesweeper
