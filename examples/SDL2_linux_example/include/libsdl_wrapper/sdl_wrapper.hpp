#ifndef SDL_WRAPPER_EXAMPLE
#define SDL_WRAPPER_EXAMPLE
#include <SDL2/SDL.h>

namespace SDLWrapper
{
	class Window
	{
	private:
		bool m_exited = false;
		SDL_Window* m_window;
		SDL_Event m_event;
		int m_width, m_height;
		char* m_title;
		bool running = true;
		bool m_init();
	public:
		const inline bool is_running() {return running;}
		void update();
		void exit();
		Window(int width, int height, char* title);
		~Window();
	};
} // namespace SDLWrapper


#endif //SDL_WRAPPER_EXAMPLE
