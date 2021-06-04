#include "game.h"
#include <stdlib.h>
#include <time.h>

signed char
getState(const int8_t *state, const uint32_t *mines,
		unsigned char row, unsigned char col)
/*
 * This function returns the number of adjacent mines to a tile i if that tile
 * is untouched.
 */
{
	unsigned char acc;
	unsigned short i = BOARD_W * row + col;

	if (state[i] != -1)
		return state[i];
	if (mines[row] >> col & 1)
		return MINE;
	acc = mines[row] >> (col + 1) & 1;
	if (row < BOARD_H - 1) {
		acc += (col && (mines[row + 1] >> (col - 1) & 1)) +
		(mines[row + 1] >> (col + 1) & 1) + (mines[row + 1] >> col & 1);
	}
	acc += (col && (mines[row] >> (col - 1) & 1));
	if (row) {
		acc += (col && (mines[row - 1] >> (col - 1) & 1)) + 
		(mines[row - 1] >> col & 1) + (mines[row - 1] >> (col + 1) & 1);
	}
	return acc;
}

void
flag(int8_t *state, unsigned char row, unsigned char col)
{
	unsigned short i = BOARD_W * row + col;

	if (state[i] == UNDEF)
		state[i] = FLAG;
	else if (state[i] == FLAG)
		state[i] = UNDEF;
}

void
newGame(game_t *game)
{
	short i;

	for (i = 0; i < BOARD_H * BOARD_W; ++i)
		game->state[i] = UNDEF;
	game->nMines = MINES;
}

void
scatter(uint32_t *mines, unsigned char nMines, unsigned char row,
		unsigned char col)
{
	short i, j, tiles = BOARD_H * BOARD_W - 9;
	char tmp;

	srand(time(NULL));
	for (i = 0; i < BOARD_H; ++i) {
		mines[i] = 0;
		for (j = 0; j < BOARD_W; ++j) {
			if (i - row <= 1 && i - row >= -1 && j - col <= 1 && j - col >= -1)
				continue;
			tmp = RAND_MAX * (uint64_t)nMines / tiles-- > rand();
			mines[i] |= tmp << j;
			nMines -= tmp;
		}
	}
}
