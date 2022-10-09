#include "sdlw.h"

SDL_Renderer* get_renderer(SDL_Window* window)
{
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if(!renderer) { puts("error getting renderer"); }
	return renderer;
}

void render(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}