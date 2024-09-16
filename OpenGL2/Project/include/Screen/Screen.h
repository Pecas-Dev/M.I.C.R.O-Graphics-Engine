#pragma once

#include <SDL.h>


class Screen
{
public:
	static Screen* Instance();

	bool Initialize();
	void ClearScreen();
	void Present();
	void Shutdown();

private:
	Screen();
	Screen(const Screen&);

	Screen& operator=(const Screen&);

	SDL_Window* window;
	SDL_GLContext context;
};

