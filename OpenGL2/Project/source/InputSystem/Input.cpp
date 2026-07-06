#include <InputSystem/Input.h>

#include<imgui_impl_sdl2.h>

#include <iostream>


Input* Input::Instance()
{
	static Input* inputObject = new Input();
	return inputObject;
}

Input::Input()
{
	m_keyUp = 0;
	m_keyDown = 0;

	m_isXClicked = false;
	m_isKeyPressed = false;

	m_isLeftButtonClicked = false;
	m_isRightButtonClicked = false;
	m_isMiddleButtonClicked = false;

	m_wasLeftButtonPressed = false;
	m_wasRightButtonPressed = false;
	m_wasMiddleButtonPressed = false;

	m_mouseMotionX = 0;
	m_mouseMotionY = 0;

	m_mousePositionX = 0;
	m_mousePositionY = 0;

	m_mouseWheel = 0;

	m_keyTapped = SDLK_UNKNOWN;

	m_isCtrlDown = false;
	m_isShiftDown = false;
	m_isAltDown = false;

	m_keyboardState = SDL_GetKeyboardState(nullptr);
}

bool Input::IsXClicked()
{
	return m_isXClicked;
}

bool Input::IsKeyPressed()
{
	return m_isKeyPressed;
}

char Input::GetKeyUp()
{
	return m_keyUp;
}

char Input::GetKeyDown()
{
	return m_keyDown;
}

bool Input::IsLeftButtonClicked()
{
	return m_isLeftButtonClicked;
}

bool Input::IsRightButtonClicked()
{
	return m_isRightButtonClicked;
}

bool Input::IsMiddleButtonClicked()
{
	return m_isMiddleButtonClicked;
}

bool Input::WasLeftButtonPressed()
{
	return m_wasLeftButtonPressed;
}

bool Input::WasRightButtonPressed()
{
	return m_wasRightButtonPressed;
}

bool Input::WasMiddleButtonPressed()
{
	return m_wasMiddleButtonPressed;
}

int Input::GetMousePositionX()
{
	return m_mousePositionX;
}

int Input::GetMousePositionY()
{
	return m_mousePositionY;
}

int Input::GetMouseMotionX()
{
	return m_mouseMotionX;
}

int Input::GetMouseMotionY()
{
	return m_mouseMotionY;
}

int Input::GetMouseWheel()
{
	return m_mouseWheel;
}

SDL_Keycode Input::GetKeyTapped()
{
	return m_keyTapped;
}

bool Input::IsCtrlDown()
{
	return m_isCtrlDown;
}

bool Input::IsShiftDown()
{
	return m_isShiftDown;
}

bool Input::IsAltDown()
{
	return m_isAltDown;
}

bool Input::IsKeyHeld(SDL_Scancode scancode)
{
	return m_keyboardState && m_keyboardState[scancode] != 0;
}

const std::string& Input::GetDroppedFile()
{
	return m_droppedFile;
}

void Input::Update()
{
	SDL_Event events;

	m_mouseMotionX = 0;
	m_mouseMotionY = 0;
	m_mouseWheel = 0;

	m_wasLeftButtonPressed = false;
	m_wasRightButtonPressed = false;
	m_wasMiddleButtonPressed = false;

	m_keyTapped = SDLK_UNKNOWN;
	m_droppedFile.clear();

	SDL_Keymod modState = SDL_GetModState();
	m_isCtrlDown = (modState & KMOD_CTRL) != 0;
	m_isShiftDown = (modState & KMOD_SHIFT) != 0;
	m_isAltDown = (modState & KMOD_ALT) != 0;

	while (SDL_PollEvent(&events))
	{
		ImGui_ImplSDL2_ProcessEvent(&events);

		switch (events.type)
		{

		case SDL_QUIT:
		{
			m_isXClicked = true;
			break;
		}

		case SDL_KEYDOWN:
		{
			m_isKeyPressed = true;
			m_keyDown = events.key.keysym.sym;

			if (events.key.repeat == 0)
			{
				m_keyTapped = events.key.keysym.sym;

				m_isCtrlDown = m_isCtrlDown || (events.key.keysym.mod & KMOD_CTRL) != 0;
				m_isShiftDown = m_isShiftDown || (events.key.keysym.mod & KMOD_SHIFT) != 0;
				m_isAltDown = m_isAltDown || (events.key.keysym.mod & KMOD_ALT) != 0;
			}

			break;
		}

		case SDL_DROPFILE:
		{
			if (events.drop.file)
			{
				m_droppedFile = events.drop.file;
				SDL_free(events.drop.file);
			}

			break;
		}

		case SDL_KEYUP:
		{
			m_isKeyPressed = false;
			m_keyUp = events.key.keysym.sym;
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		{

			switch (events.button.button)
			{

			case SDL_BUTTON_LEFT:
			{
				m_isLeftButtonClicked = true;
				m_wasLeftButtonPressed = true;
				break;
			}

			case SDL_BUTTON_RIGHT:
			{
				m_isRightButtonClicked = true;
				m_wasRightButtonPressed = true;
				break;
			}

			case SDL_BUTTON_MIDDLE:
			{
				m_isMiddleButtonClicked = true;
				m_wasMiddleButtonPressed = true;
				break;
			}

			}

			break;
		}

		case SDL_MOUSEBUTTONUP:
		{
			switch (events.button.button)
			{

			case SDL_BUTTON_LEFT:
			{
				m_isLeftButtonClicked = false;
				break;
			}

			case SDL_BUTTON_RIGHT:
			{
				m_isRightButtonClicked = false;
				break;
			}

			case SDL_BUTTON_MIDDLE:
			{
				m_isMiddleButtonClicked = false;
				break;
			}

			}
			break;
		}

		case SDL_MOUSEMOTION:
		{
			m_mouseMotionX += events.motion.xrel;
			m_mouseMotionY += events.motion.yrel;
			break;
		}

		case SDL_MOUSEWHEEL:
		{
			m_mouseWheel += events.wheel.y;
			break;
		}

		}
	}

	SDL_GetMouseState(&m_mousePositionX, &m_mousePositionY);
}
