#pragma once

#include <SDL.h>

#include <string>


class Input
{

public:
	static Input* Instance();

	bool IsXClicked();
	bool IsKeyPressed();

	char GetKeyUp();
	char GetKeyDown();

	SDL_Keycode GetKeyTapped();

	bool IsCtrlDown();
	bool IsShiftDown();
	bool IsAltDown();

	bool IsKeyHeld(SDL_Scancode scancode);

	const std::string& GetDroppedFile();

	bool IsLeftButtonClicked();
	bool IsRightButtonClicked();
	bool IsMiddleButtonClicked();

	bool WasLeftButtonPressed();
	bool WasRightButtonPressed();
	bool WasMiddleButtonPressed();

	int GetMousePositionX();
	int GetMousePositionY();

	int GetMouseMotionX();
	int GetMouseMotionY();

	int GetMouseWheel();

	void Update();

private:
	Input();
	Input(const Input&);

	Input& operator=(Input&);

	char m_keyUp;
	char m_keyDown;

	bool m_isXClicked;
	bool m_isKeyPressed;

	bool m_isLeftButtonClicked;
	bool m_isRightButtonClicked;
	bool m_isMiddleButtonClicked;

	bool m_wasLeftButtonPressed;
	bool m_wasRightButtonPressed;
	bool m_wasMiddleButtonPressed;

	int m_mouseMotionX;
	int m_mouseMotionY;

	int m_mousePositionX;
	int m_mousePositionY;

	int m_mouseWheel;

	SDL_Keycode m_keyTapped;

	bool m_isCtrlDown;
	bool m_isShiftDown;
	bool m_isAltDown;

	const Uint8* m_keyboardState;

	std::string m_droppedFile;
};

