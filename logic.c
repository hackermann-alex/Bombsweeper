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
	unsigned char acc = 0, i = BOARD_W * row + col;

	if (state[i] != -1)
		return state[i];
	if (mines[row] >> col & 1)
		return MINE;
	acc += mines[row] >> (col + 1) & 1;
	acc += mines[row + 1] >> (col + 1) & 1;
	acc += mines[row + 1] >> col & 1;
	acc += mines[row + 1] >> (col - 1) & 1;
	acc += mines[row] >> (col - 1) & 1;
	acc += mines[row - 1] >> (col - 1) & 1;
	acc += mines[row - 1] >> col & 1;
	return acc + (mines[row - 1] >> (col + 1) & 1);
}

void
newGame(game_t *game, unsigned char nMines)
{
	short i, j, tiles = BOARD_H * BOARD_W;
	char tmp;

	for (i = 0; i < tiles; ++i)
		game->state[i] = UNDEF;
	game->nMines = nMines;
	srand(time(NULL));
	for (i = 0; i < BOARD_H; ++i) {
		game->mines[i] = 0;
		for (j = 0; j < BOARD_W; ++j) {
			tmp = RAND_MAX * (uint64_t)nMines / tiles-- >= rand();
			game->mines[i] |= tmp << j;
			nMines -= tmp;
		}
	}
}