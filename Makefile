OBJ = main.o render.o logic.o
DEST = game
INCS = -lSDL2 -lSDL2_gfx
CFLAGS = -std=c99 -Wall -O3 -pedantic
CC = cc

all: ${OBJ}
	${CC} ${INCS} -o ${DEST} ${OBJ}

main.o: main.c game.h
	${CC} ${CFLAGS} -c -o $@ $<

render.o: render.c game.h
	${CC} ${CFLAGS} -c -o $@ $<

logic.o: logic.c game.h
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm *.o
