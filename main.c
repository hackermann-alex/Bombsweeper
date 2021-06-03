#include "game.h"
#include <SDL2/SDL.h>

void gameLoop();

int
main()
{
	unsigned char code = init();

	gameLoop();
	quit(code);
}
