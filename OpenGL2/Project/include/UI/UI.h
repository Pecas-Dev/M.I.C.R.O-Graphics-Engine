#pragma once

#include <Utility/Utility.h>
#include <Shape/Model.h>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <string>
#include <vector>
#include <memory>
#include <deque>


class Model;


class UI
{
public:
	UI(int screenWidth, int screenHeight, int consoleWindowHeight, int propertiesWindowWidth, std::deque<std::string>& messages, std::vector<std::unique_ptr<Model>>& models, bool& isLit);

	void RenderConsoleWindow();
	void RenderPropertiesWindow();

private:
	std::vector<std::unique_ptr<Model>>& m_models;
	std::deque<std::string>& m_messages;

	int m_screenWidth;
	int m_screenHeight;
	int m_consoleWindowHeight;
	int m_propertiesWindowWidth;

	bool& m_isLit;
};
