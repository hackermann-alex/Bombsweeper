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
static unsigned char scale;
static unsigned short originX;
static unsigned short originY;

void
renderTile(const int8_t state, unsigned char row, unsigned char col)
{
	const static SDL_Rect clips[3] = { { 0, CHAR_SZ, TILE_SZ, TILE_SZ },
		{TILE_SZ, CHAR_SZ, TILE_SZ, TILE_SZ },
		{ TILE_SZ << 1, CHAR_SZ, TILE_SZ, TILE_SZ} };
	SDL_Rect num = { 0, 0, CHAR_SZ, CHAR_SZ },
		 renderQuad = { originX + scale * col * TILE_SZ,
			 originY + scale * row * TILE_SZ, scale * TILE_SZ,  scale * TILE_SZ };

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
		renderQuad.y += scale;
		SDL_RenderCopy(renderer, sprites, &num, &renderQuad);
	}
}

void
renderScene(const int8_t *state)
{
	unsigned char i, j;

	for (i = 0; i < BOARD_H; ++i) {
		for (j = 0; j < BOARD_W; ++j)
			renderTile(state[BOARD_W * i + j], i, j);
	}
}

void
getTile(int sx, int sy, signed char *x, signed char *y)
{
	sx = (sx - originX) / (scale * TILE_SZ);
	sy = (sy - originY) / (scale * TILE_SZ);
	*x = (sx < 0 || sx >= BOARD_W) ? -1 : sx;
	*y = (sy < 0 || sy >= BOARD_H) ? -1 : sy;
}

void
gameLoop()
{
	SDL_Event e;
	game_t game;
	signed char x, y;

	newGame(&game, MINES);
	renderScene(game.state);
	SDL_RenderPresent(renderer);
loop:
	while (SDL_WaitEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			return;
		case SDL_MOUSEBUTTONDOWN:
			getTile(e.button.x, e.button.y, &x, &y);
			game.state[BOARD_W * y + x] =
				getState(game.state, game.mines, y, x);
			renderTile(game.state[BOARD_W * y + x], y, x);
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
	scale = MIN(scaleX, scaleY);
	originX = (w - scale * BOARD_W * TILE_SZ) >> 1;
	originY = (h - scale * BOARD_H * TILE_SZ) >> 1;
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
