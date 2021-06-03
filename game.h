#ifndef DRAWING_H
#define DRAWING_H

#include <stdint.h>

#define BOARD_H 15
#define BOARD_W 30
#define MINES 99

enum states { MINE = -3, FLAG = -2, UNDEF = -1 };

typedef struct {
	int8_t state[BOARD_H * BOARD_W];
	uint32_t mines[BOARD_H];
	unsigned char nMines;
} game_t;

unsigned char init();
void quit(unsigned char code);
void gameLoop();
void newGame(game_t *game, unsigned char nMines);
signed char getState(const int8_t *state, const uint32_t *mines,
		unsigned char row, unsigned char col);

#endif
