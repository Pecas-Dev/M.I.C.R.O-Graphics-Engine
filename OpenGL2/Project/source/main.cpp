#include <Transform/Transform.h>
#include <InputSystem/Input.h>
#include <Environment/Environment.h>
#include <PostProcess/PostProcess.h>
#include <RenderTarget/RenderTarget.h>
#include <Utility/Utility.h>
#include <Objects/Object.h>
#include <Camera/Camera.h>
#include <Screen/Screen.h>
#include <Shader/Shader.h>
#include <Shadow/ShadowMap.h>
#include <Shape/Model.h>
#include <Light/Light.h>
#include <Shape/Quad.h>
#include <Shape/Cube.h>
#include <Grid/Grid.h>
#include <UI/UI.h>

#include <SDL.h>
#include <SDL_image.h>
#include <imgui.h>
#include <glad/gl.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <ctime>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
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


static void SaveViewportScreenshot(const RenderTarget& target)
{
	const int width = target.GetWidth();
	const int height = target.GetHeight();

	std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 4);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	std::vector<unsigned char> flipped(pixels.size());
	const size_t rowBytes = static_cast<size_t>(width) * 4;

	for (int row = 0; row < height; row++)
	{
		std::memcpy(&flipped[row * rowBytes], &pixels[(height - 1 - row) * rowBytes], rowBytes);
	}

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(flipped.data(), width, height, 32, static_cast<int>(rowBytes), SDL_PIXELFORMAT_RGBA32);

	if (!surface)
	{
		Utility::AddMessage("Screenshot failed: could not create surface");
		return;
	}

	std::error_code errorCode;
	std::filesystem::create_directories("Screenshots", errorCode);

	char filename[128];
	std::time_t now = std::time(nullptr);
	std::tm localTime;
	localtime_s(&localTime, &now);
	std::strftime(filename, sizeof(filename), "Screenshots/MICRO_%Y-%m-%d_%H-%M-%S.png", &localTime);

	if (IMG_SavePNG(surface, filename) == 0)
	{
		Utility::AddMessage(std::string("Screenshot saved to ") + filename);
	}
	else
	{
		Utility::AddMessage("Screenshot failed: could not write PNG");
	}

	SDL_FreeSurface(surface);
}


int main(int argc, char* argv[])
{
	std::error_code cwdError;

	if (!std::filesystem::exists("Assets", cwdError))
	{
		if (char* basePath = SDL_GetBasePath())
		{
			std::filesystem::current_path(basePath, cwdError);
			SDL_free(basePath);
		}
	}

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

	lightShader.Create("Assets/Shaders/Light.vert", "Assets/Shaders/Light.frag");
	defaultShader.Create("Assets/Shaders/Default.vert", "Assets/Shaders/Default.frag");

	Shader depthShader;
	depthShader.Create("Assets/Shaders/Depth.vert", "Assets/Shaders/Depth.frag");

	ShadowMap shadowMap;
	shadowMap.Create(2048);

	Environment environment;
	environment.Load("Assets/HDRI/kloppenheim_06_puresky_2k.hdr");

	const int viewportWidth = SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH;
	const int viewportHeight = SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT;

	RenderTarget sceneTarget;
	sceneTarget.Create(viewportWidth, viewportHeight, GL_RGBA16F, true);

	PostProcess post;
	post.Create(viewportWidth, viewportHeight);

	camera.Set3DView(58.0f, (float)viewportWidth / (float)viewportHeight);

	ImGui::GetIO().ConfigDragClickToInputText = true;

	ImFont* mainFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Assets/Fonts/CascadiaCode.ttf", 15.0f);
	ImGui::GetIO().FontDefault = mainFont;


	UI ui(SCREEN_WIDTH, SCREEN_HEIGHT, CONSOLE_WINDOW_HEIGHT, PROPERTIES_WINDOW_WIDTH, messages, models, isLit, light, &grid, camera, environment, post);
	ui.SetFonts(mainFont, mainFont);

	if (argc > 1)
	{
		ui.LoadModel(argv[1]);
	}


	Uint64 previousCounter = SDL_GetPerformanceCounter();

	while (isAppRunning)
	{
		Uint64 currentCounter = SDL_GetPerformanceCounter();
		float deltaTime = (float)(currentCounter - previousCounter) / (float)SDL_GetPerformanceFrequency();
		previousCounter = currentCounter;

		if (deltaTime > 0.1f) { deltaTime = 0.1f; } 

		Input::Instance()->Update();

		ui.Update(deltaTime);
		camera.UpdateMotion(deltaTime);

		camera.HandleNavigation(deltaTime, ui.IsViewportHovered() && !ui.IsGizmoBusy());

		if (camera.IsNavigating())
		{
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}
		else
		{
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}


		isAppRunning = !Input::Instance()->IsXClicked();

		int panelWidth = 0;
		int panelHeight = 0;
		ui.GetViewportSize(panelWidth, panelHeight);

		sceneTarget.Resize(panelWidth, panelHeight);
		post.Resize(sceneTarget.GetWidth(), sceneTarget.GetHeight());
		camera.Set3DView(58.0f, (float)sceneTarget.GetWidth() / (float)sceneTarget.GetHeight());

		if (isLit)
		{
			shadowMap.BeginDepthPass(light.GetTransform().GetPosition());
			depthShader.UseShader();

			for (auto& model : models)
			{
				if (!model->Visible()) { continue; }

				model->RenderDepth(depthShader);
			}

			shadowMap.EndDepthPass();
		}


		sceneTarget.Bind();
		Screen::Instance()->ClearScreen();

		defaultShader.UseShader();
		camera.SendToShader(defaultShader);

		grid.Render(defaultShader);

		if (isLit)
		{
			light.Render(defaultShader);
		}

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
			if (!model->Visible()) { continue; }

			if (isLit)
			{
				lightShader.UseShader();
				light.SendToShader(lightShader);
				camera.SendToShader(lightShader);
				shadowMap.SendToShader(lightShader);
				environment.SendToShader(lightShader);
				model->Render(lightShader);
			}
			else
			{
				defaultShader.UseShader();
				camera.SendToShader(defaultShader);
				model->Render(defaultShader);
			}
		}

		environment.RenderSkybox(camera.GetViewMatrix(), camera.GetProjectionMatrix());

		sceneTarget.Unbind();

		GLuint finalTexture = post.Apply(sceneTarget.GetColorTexture());

		if (ui.ConsumeScreenshotRequest())
		{
			post.GetOutput().Bind();
			SaveViewportScreenshot(post.GetOutput());
			post.GetOutput().Unbind();
		}

		int drawableWidth = 0;
		int drawableHeight = 0;
		Screen::Instance()->GetDrawableSize(drawableWidth, drawableHeight);

		glViewport(0, 0, drawableWidth, drawableHeight);
		Screen::Instance()->ClearScreen();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ui.RenderDockspace();
		ui.RenderSceneWindow(finalTexture);
		ui.RenderOutlinerWindow();
		ui.RenderConsoleWindow();
		ui.RenderPropertiesWindow();
		ui.RenderOverlays();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		Screen::Instance()->Present();
	}

	sceneTarget.Destroy();
	post.Destroy();
	environment.Destroy();
	shadowMap.Destroy();
	depthShader.Destroy();
	lightShader.Destroy();
	Screen::Instance()->Shutdown();

	return 0;
}