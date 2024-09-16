#include <Transform/Transform.h>
#include <InputSystem/Input.h>
#include <Utility/Utility.h>
#include <Objects/Object.h>
#include <Camera/Camera.h>
#include <Screen/Screen.h>
#include <Shader/Shader.h>
#include <Shape/Model.h>
#include <Light/Light.h>
#include <Shape/Quad.h>
#include <Shape/Cube.h>
#include <Grid/Grid.h>
#include <UI/UI.h>

#include <SDL.h>
#include <imgui.h>
#include <glad/gl.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <memory>
#include <deque>


const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int CONSOLE_WINDOW_HEIGHT = 250;
const int PROPERTIES_WINDOW_WIDTH = 400;

bool isLit = false;
bool isAppRunning = true;

std::vector<std::unique_ptr<Object>> objects;
std::vector<std::unique_ptr<Model>> models;
std::deque<std::string> messages;


int main(int argc, char* argv[])
{
	if (!Screen::Instance()->Initialize()) { return 0; }

	Shader defaultShader;
	Shader lightShader;
	Camera camera;
	Light light;
	Grid grid;


	//#CUBE 1
	//objects.push_back(std::make_unique<Cube>("Crate_2.png", &grid));

	//#CUBE 2	  
	//objects.push_back(std::make_unique<Cube>("Crate_1.png", &grid));

	//#MODELS
	models.push_back((std::make_unique<Model>(&grid)));
	models.back()->Load("Assets/Models/Armchair.obj");


	lightShader.Create("Assets/Shaders/Light.vert", "Assets/Shaders/Light.frag");
	defaultShader.Create("Assets/Shaders/Default.vert", "Assets/Shaders/Default.frag");

	camera.Set3DView();
	camera.SetViewPort(0, CONSOLE_WINDOW_HEIGHT, SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH, SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT);
	camera.SetMoveSpeed(0.25f);
	camera.SetZoomSpeed(0.75f);
	camera.SetRotationSpeed(0.85f);

	light.SetSpeed(0.1f);


	ImGui::GetIO().Fonts->AddFontFromFileTTF("Assets/Fonts/CascadiaCode.ttf", 14.0f);
	ImGui::GetIO().Fonts->Build();


	SDL_Rect mouseCollider = { 0 };
	SDL_Rect sceneCollider = { 0, 0, SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH, SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT };


	UI ui(SCREEN_WIDTH, SCREEN_HEIGHT, CONSOLE_WINDOW_HEIGHT, PROPERTIES_WINDOW_WIDTH, messages, models, isLit);


	while (isAppRunning)
	{
		Screen::Instance()->ClearScreen();
		Input::Instance()->Update();

		mouseCollider = { Input::Instance()->GetMousePositionX(), Input::Instance()->GetMousePositionY(), 1, 1 };

		bool isMouseColliding = SDL_HasIntersection(&mouseCollider, &sceneCollider);

		light.MoveLight(light);
		camera.ZoomCamera(camera);

		if (isMouseColliding)
		{
			grid.MoveGrid(grid);
		}


		isAppRunning = !Input::Instance()->IsXClicked();


		defaultShader.UseShader();
		camera.SendToShader(defaultShader);

		grid.Render(defaultShader);
		light.Render(defaultShader);

		for (auto& object : objects)
		{
			if (isLit)
			{
				lightShader.UseShader();
				light.SendToShader(lightShader);
				camera.SendToShader(lightShader);
				object->Render(lightShader);
			}
			else
			{
				defaultShader.UseShader();
				camera.SendToShader(defaultShader);
				object->Render(defaultShader);
			}
		}

		for (auto& model : models)
		{
			if (isLit)
			{
				lightShader.UseShader();
				light.SendToShader(lightShader);
				camera.SendToShader(lightShader);
				model->Render(lightShader);
			}
			else
			{
				defaultShader.UseShader();
				camera.SendToShader(defaultShader);
				model->Render(defaultShader);
			}
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ui.RenderConsoleWindow();
		ui.RenderPropertiesWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		Screen::Instance()->Present();
	}

	lightShader.Destroy();
	Screen::Instance()->Shutdown();

	return 0;
}