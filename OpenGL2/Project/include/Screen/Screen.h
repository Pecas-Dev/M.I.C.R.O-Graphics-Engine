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

	void GetDrawableSize(int& width, int& height);

private:
	Screen();
	Screen(const Screen&);

	Screen& operator=(const Screen&);

	SDL_Window* window;
	SDL_GLContext context;
};

