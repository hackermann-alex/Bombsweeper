#include <SDL2/SDL.h>
#include "game.h"

#define NAME "Bombsweeper"
#define BG 0x47, 0x80, 0x61, 0xFF 
#define COLOUR 0xFF, 0xFF, 0xFF, 0xFF
#define SHEET "assets.bmp"
#define KEY 0xFF, 0xFF, 0xFF
#define CHAR_SZ 8
#define TILE_SZ 16

#define MIN(a, b) ((a) < (b) ? (a) : (b))

enum codes { SUCCESS, INIT_SDL, RENDERER, IMG, TEX, WINDOW };

static SDL_Renderer *renderer;
static SDL_Window *window;
static SDL_Texture *sprites;
static unsigned char scaled;
static unsigned short originX;
static unsigned short originY;

void
renderTile(const int8_t state, unsigned char row, unsigned char col)
{
	const static SDL_Rect clips[3] = { { 0, CHAR_SZ, TILE_SZ, TILE_SZ },
		{TILE_SZ, CHAR_SZ, TILE_SZ, TILE_SZ },
		{ TILE_SZ << 1, CHAR_SZ, TILE_SZ, TILE_SZ} };
	SDL_Rect num = { 0, 0, CHAR_SZ, CHAR_SZ },
		 renderQuad = { originX + scaled * col,
			 originY + scaled * row, scaled,  scaled };

	switch (state) {
	case UNDEF:
		SDL_RenderCopy(renderer, sprites, clips, &renderQuad);
		break;
	case FLAG:
		SDL_RenderCopy(renderer, sprites, clips, &renderQuad);
		SDL_RenderCopy(renderer, sprites, clips + 1, &renderQuad);
		break;
	case MINE:
		SDL_RenderCopy(renderer, sprites, clips + 2, &renderQuad);
		break;
	case 0:
		SDL_SetRenderDrawColor(renderer, 0xBF, 0xBF, 0xBF, 0xFF);
		SDL_RenderFillRect(renderer, &renderQuad);
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawRect(renderer, &renderQuad);
		break;
	default:
		num.x = state * CHAR_SZ;
		SDL_SetRenderDrawColor(renderer, 0xBF, 0xBF, 0xBF, 0xFF);
		SDL_RenderFillRect(renderer, &renderQuad);
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawRect(renderer, &renderQuad);
		renderQuad.y += scaled / TILE_SZ;
		SDL_RenderCopy(renderer, sprites, &num, &renderQuad);
	}
}

void
renderScene()
{
	unsigned char i, j;

	for (i = 0; i < BOARD_H; ++i) {
		for (j = 0; j < BOARD_W; ++j)
			renderTile(UNDEF, i, j);
	}
}

void
getTile(int sx, int sy, signed char *x, signed char *y)
{
	sx = (sx - originX) / scaled;
	sy = (sy - originY) / scaled;
	*x = (sx < 0 || sx >= BOARD_W) ? -1 : sx;
	*y = (sy < 0 || sy >= BOARD_H) ? -1 : sy;
}

void
open(int8_t *state, const uint32_t *mines,
		unsigned char row, unsigned char col)
{
	unsigned short i = BOARD_W * row + col;
	state[i] = getState(state, mines, row, col);
	renderTile(state[i], row, col);

	if (!state[i]) {
		if (state[i + 1] == UNDEF && col < BOARD_W - 1)
			open(state, mines, row, col + 1);
		if (state[i - BOARD_W + 1] == UNDEF && row && col < BOARD_W - 1)
			open(state, mines, row - 1, col + 1);
		if (state[i - BOARD_W] == UNDEF && row)
			open(state, mines, row - 1, col);
		if (state[i - BOARD_W - 1] == UNDEF && row && col)
			open(state, mines, row - 1, col - 1);
		if (state[i - 1] == UNDEF && col)
			open(state, mines, row, col - 1);
		if (state[i + BOARD_W - 1] == UNDEF && row < BOARD_H - 1 && col)
			open(state, mines, row + 1, col - 1);
		if (state[i + BOARD_W] == UNDEF && row < BOARD_H - 1)
			open(state, mines, row + 1, col);
		if (state[i + BOARD_W + 1] == UNDEF && row < BOARD_H - 1 && col < BOARD_W - 1)
			open(state, mines, row + 1, col + 1);
	}
}

void
gameLoop()
{
	SDL_Event e;
	game_t game;
	signed char x, y;

	newGame(&game);
	renderScene();
	SDL_RenderPresent(renderer);
start:
	while (SDL_WaitEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			return;
		case SDL_MOUSEBUTTONDOWN:
			getTile(e.button.x, e.button.y, &x, &y);
			if (x < 0 || y < 0)
				goto start;
			switch (e.button.button) {
			case SDL_BUTTON_LEFT:
				scatter(game.mines, MINES, y, x);
				open(game.state, game.mines, y, x);
				SDL_RenderPresent(renderer);
				goto loop;
			case SDL_BUTTON_RIGHT:
				flag(game.state, y, x);
				renderTile(game.state[BOARD_W * y + x], y, x);
				SDL_RenderPresent(renderer);
			}
		}
	}
	goto start;
loop:
	while (SDL_WaitEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			return;
		case SDL_MOUSEBUTTONDOWN:
			getTile(e.button.x, e.button.y, &x, &y);
			if (x < 0 || y < 0)
				goto loop;
			switch (e.button.button) {
			case SDL_BUTTON_LEFT:
				open(game.state, game.mines, y, x);
				break;
			case SDL_BUTTON_RIGHT:
				flag(game.state, y, x);
				renderTile(game.state[BOARD_W * y + x], y, x);
			}
			SDL_RenderPresent(renderer);
		}
	}
	goto loop;
}

unsigned char
init()
{
	int w, h;
	unsigned char scaleX, scaleY;
	SDL_Surface *loadSurface;

	/* Init SDL */
	if (SDL_Init(SDL_INIT_VIDEO))
		return INIT_SDL;
	/* Create window */
	window = SDL_CreateWindow(NAME,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0,
			SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (!window)
		return WINDOW;
	/* Create renderer */
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		return RENDERER;
	/* Load sprite sheet */
	if (!(loadSurface = SDL_LoadBMP(SHEET)))
		return IMG;
	SDL_SetColorKey(loadSurface, SDL_TRUE, SDL_MapRGB(loadSurface->format, KEY));
	sprites = SDL_CreateTextureFromSurface(renderer, loadSurface);
	SDL_FreeSurface(loadSurface);
	if (!sprites)
		return TEX;
	/* Render scene */
	SDL_SetRenderDrawColor(renderer, BG);
	SDL_RenderClear(renderer);

	SDL_GetWindowSize(window, &w, &h);
	scaleX = w / (BOARD_W * TILE_SZ);
	scaleY = h / (BOARD_H * TILE_SZ);
	scaled = TILE_SZ * MIN(scaleX, scaleY);
	originX = (w - scaled * BOARD_W) >> 1;
	originY = (h - scaled * BOARD_H) >> 1;
	return SUCCESS;
}

void
quit(unsigned char code)
{
	switch (code) {
	case SUCCESS:
		SDL_DestroyTexture(sprites);
	case TEX:
	case IMG:
		SDL_DestroyRenderer(renderer);
	case RENDERER:
		SDL_DestroyWindow(window);
	case WINDOW:
		SDL_Quit();
	default:
		exit(code);
	}
}
