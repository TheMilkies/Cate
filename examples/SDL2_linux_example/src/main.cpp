#include "libsdl_wrapper/sdl_wrapper.hpp"

int main(int argc, char const *argv[])
{
	SDLWrapper::Window window(640, 480, "SDL Wrapper example");

	while (window.is_running())
	{
		window.update();
	}
	
	return 0;
}
