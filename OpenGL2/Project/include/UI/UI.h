#pragma once

#include <Utility/Utility.h>
#include <Camera/Camera.h>
#include <Environment/Environment.h>
#include <PostProcess/PostProcess.h>
#include <Shape/Model.h>
#include <Light/Light.h>
#include <Grid/Grid.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <deque>


class Model;


class UI
{
public:
	UI(int screenWidth, int screenHeight, int consoleWindowHeight, int propertiesWindowWidth, std::deque<std::string>& messages, std::vector<std::unique_ptr<Model>>& models, bool& isLit, Light& light, Grid* parentGrid, Camera& camera, Environment& environment, PostProcess& post);

	void Update(float deltaTime);

	void RenderDockspace();

	void RenderSceneWindow(unsigned int colorTexture);

	void RenderConsoleWindow();
	void RenderPropertiesWindow();
	void RenderOutlinerWindow();

	void RenderOverlays();

	bool IsViewportHovered() const;
	bool IsGizmoBusy() const;

	void GetViewportSize(int& width, int& height) const;

	bool ConsumeScreenshotRequest();

	void LoadModel(const std::string& filePath);

	void SetFonts(ImFont* titleFont, ImFont* headingFont);

	static void ApplyStyle(bool lightTheme = false);

private:
	struct PaletteAction
	{
		std::string label;
		std::string shortcut;
		std::function<void()> run;
	};

	static bool SectionHeader(const char* label);

	static void Hint(const char* text);

	bool AxisDragRow(const char* label, glm::vec3& value, float speed, float min, float max, float resetValue, const char* hint);
	bool SliderDrag(const char* label, float* value, float min, float max, const char* format);
	bool ColorPickerRow(const char* label, float* color, bool hasAlpha);

	void PollConsoleMessages();
	void OpenPalette();
	void BuildPaletteActions();

	void RenderStatsOverlay();
	void RenderEmptyState();
	void RenderCommandPalette();
	void RenderShortcutSheet();
	void RenderGizmo();
	void RenderGizmoToolbar();

	void HandleViewportPicking();
	void PickModel(float mouseX, float mouseY);

	void RemoveModel(int index);

	void RefreshSkyboxList();

	void RenderTransformSection();
	void RenderMaterialSection();
	void RenderCameraSection();
	void RenderEnvironmentSection();
	void RenderPostSection();
	void RenderLightSection();
	void RenderLoaderSection();

	Model* CurrentModel();

	std::vector<std::unique_ptr<Model>>& m_models;
	std::deque<std::string>& m_messages;

	int m_screenWidth;
	int m_screenHeight;
	int m_consoleWindowHeight;
	int m_propertiesWindowWidth;

	bool& m_isLit;

	Light& m_light;

	Grid* m_parentGrid;

	Camera& m_camera;

	Environment& m_environment;

	PostProcess& m_post;

	ImFont* m_titleFont = nullptr;
	ImFont* m_headingFont = nullptr;

	std::vector<PaletteAction> m_paletteActions;
	char m_paletteQuery[128] = { 0 };
	int m_paletteSelection = 0;
	bool m_paletteOpen = false;
	bool m_paletteJustOpened = false;

	bool m_showStats = true;
	bool m_showShortcuts = false;

	int m_selectedModel = -1;

	int m_gizmoOperation = 0;
	bool m_gizmoBusy = false;
	bool m_snapEnabled = false;

	Texture m_iconMove;
	Texture m_iconRotate;
	Texture m_iconScale;
	bool m_iconsLoaded = false;

	float m_statsHeight = 0.0f;

	bool m_leftDownLast = false;
	bool m_clickStartedInViewport = false;
	float m_clickTravel = 0.0f;

	bool m_lightTheme = false;

	float m_emptyStateAlpha = 1.0f;

	std::vector<std::string> m_skyboxFiles;

	bool m_turntable = false;
	float m_turntableSpeed = 20.0f; 

	bool m_easeGridHome = false;

	bool m_screenshotRequested = false;

	float m_deltaTime = 0.0f;

	ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f);
	ImVec2 m_viewportSize = ImVec2(880.0f, 470.0f);
	bool m_viewportHovered = false;
	bool m_viewportHoveredLastFrame = false;
	ImVec2 m_clickPos = ImVec2(0.0f, 0.0f);
};
