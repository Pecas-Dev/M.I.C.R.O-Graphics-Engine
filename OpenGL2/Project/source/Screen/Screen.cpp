#include <Screen/Screen.h>

#include <imgui.h>
#include <glad/gl.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <iostream>


Screen* Screen::Instance()
{
	static Screen* screen = new Screen;
	return screen;
}

Screen::Screen()
{
	window = nullptr;
	context = nullptr;
}

bool Screen::Initialize()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		std::cout << "Error initializing SDL" << std::endl;
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	window = SDL_CreateWindow("M.I.C.R.O Graphics Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL);

	if (!window)
	{
		std::cout << "Error creating SDL window." << std::endl;
		return false;
	}

	context = SDL_GL_CreateContext(window);

	if (!context)
	{
		std::cout << "Error creating OpenGL context." << std::endl;
		return false;
	}

	if (!gladLoaderLoadGL())
	{
		std::cout << "Error loading extensions!" << std::endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ImGui::CreateContext();
	if (!ImGui_ImplOpenGL3_Init("#version 460")) { std::cout << "Error initializing ImGui-OpenGL!" << std::endl; return false; }
	if (!ImGui_ImplSDL2_InitForOpenGL(window, context)) { std::cout << "Error initializing ImGui-SDL2!" << std::endl; return false; }

	return true;
}

void Screen::ClearScreen()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Screen::Present()
{
	SDL_GL_SwapWindow(window);
}

void Screen::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}