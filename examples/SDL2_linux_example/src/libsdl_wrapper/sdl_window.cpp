#include "libsdl_wrapper/sdl_wrapper.hpp"

namespace SDLWrapper
{
	bool Window::m_init()
	{
		return SDL_Init(SDL_INIT_EVERYTHING) == 0;
	}

	Window::Window(int width, int height, char* title) : m_width(width), m_height(height), m_title(title)
	{
		if (!m_init())
			exit();

		m_window = SDL_CreateWindow(m_title, NULL, NULL, m_width, m_height, NULL);
		if (!m_window)
			exit();
	}

	void Window::exit()
	{
		if (m_exited)
			return;
		
		running = false;
		m_exited = true;
		
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	Window::~Window()
	{
		
	}

	void Window::update()
	{
		SDL_PollEvent(&m_event);
		switch (m_event.type)
		{
		case SDL_QUIT:
			exit();
			break;
		
		default:
			break;
		}
	}
}