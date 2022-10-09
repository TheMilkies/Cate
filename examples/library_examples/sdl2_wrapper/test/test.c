#include "sdlw.h"

int main(int argc, char const *argv[])
{
	SDL_Window* win = create_window("Test", 256, 256);
	SDL_Renderer* ren = get_renderer(win);
	if(!win || !ren) { quit(win, ren); return 1; }

	// Select the color for drawing. It is set to red here.
	render(ren);
	SDL_Delay(3000);

	SDL_DestroyWindow(win);
	quit(win, ren);
	return 0;
}
