#include "sdlw.h"

SDL_Window* create_window(const char* title, int w, int h)
{
	SDL_Window* window;
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		puts("error initing sdl2");
		return NULL;
	}

	window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
	
	return window;
}