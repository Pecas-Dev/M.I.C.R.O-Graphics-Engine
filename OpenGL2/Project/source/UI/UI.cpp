#include <Utility/FileDialog.h>
#include <InputSystem/Input.h>
#include <Grid/Grid.h>
#include <UI/UI.h>

#include <imgui_internal.h>
#include <vendor/ImGuizmo/ImGuizmo.h>

#include <gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>


namespace
{
	const ImVec4 kAccent(0.259f, 0.522f, 0.957f, 1.0f);
	const ImVec4 kAccentHover(0.376f, 0.647f, 0.973f, 1.0f);

	bool gLightThemeActive = false;
	const ImVec4 kSuccess(0.275f, 0.655f, 0.345f, 1.0f);
	const ImVec4 kWarning(0.921f, 0.702f, 0.231f, 1.0f);
	const ImVec4 kError(0.898f, 0.282f, 0.302f, 1.0f);
	const ImVec4 kDim(0.604f, 0.604f, 0.647f, 1.0f);

	struct MaterialPresetInfo
	{
		const char* name;
		glm::vec3 tint;
		float roughness;
		float metallic;
	};

	const MaterialPresetInfo kMaterialPresets[] =
	{
		{ "Gold",    glm::vec3(1.00f, 0.78f, 0.34f), 0.25f, 1.0f },
		{ "Silver",  glm::vec3(0.96f, 0.95f, 0.92f), 0.15f, 1.0f },
		{ "Copper",  glm::vec3(0.95f, 0.64f, 0.54f), 0.30f, 1.0f },
		{ "Chrome",  glm::vec3(0.88f, 0.90f, 0.94f), 0.06f, 1.0f },
		{ "Plastic", glm::vec3(0.92f, 0.92f, 0.92f), 0.35f, 0.0f },
		{ "Rubber",  glm::vec3(0.18f, 0.18f, 0.18f), 0.90f, 0.0f },
		{ "Clay",    glm::vec3(0.79f, 0.55f, 0.44f), 0.85f, 0.0f },
	};

	int ClassifyMessage(const std::string& message)
	{
		std::string lowered = message;

		for (auto& character : lowered)
		{
			character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
		}

		if (lowered.find("fail") != std::string::npos ||
			lowered.find("error") != std::string::npos ||
			lowered.find("unable") != std::string::npos ||
			lowered.find("missing") != std::string::npos)
		{
			return 2;
		}

		if (lowered.find("success") != std::string::npos ||
			lowered.find("loaded") != std::string::npos ||
			lowered.find("saved") != std::string::npos)
		{
			return 1;
		}

		return 0;
	}

	std::string FormatThousands(int value)
	{
		std::string digits = std::to_string(value);
		std::string result;

		int count = 0;

		for (int i = static_cast<int>(digits.size()) - 1; i >= 0; i--)
		{
			result.insert(result.begin(), digits[i]);

			if (++count % 3 == 0 && i > 0)
			{
				result.insert(result.begin(), ',');
			}
		}

		return result;
	}

	void CenteredText(const char* text, const ImVec4& color)
	{
		float width = ImGui::CalcTextSize(text).x;
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - width) * 0.5f);
		ImGui::TextColored(color, "%s", text);
	}

	bool RayIntersectsBox(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& boundsMin, const glm::vec3& boundsMax, float& hitDistance)
	{
		float tMin = 0.0f;
		float tMax = 1e30f;

		for (int axis = 0; axis < 3; axis++)
		{
			if (std::fabs(direction[axis]) < 1e-8f)
			{
				if (origin[axis] < boundsMin[axis] || origin[axis] > boundsMax[axis]) { return false; }
				continue;
			}

			float inverse = 1.0f / direction[axis];
			float tNear = (boundsMin[axis] - origin[axis]) * inverse;
			float tFar = (boundsMax[axis] - origin[axis]) * inverse;

			if (tNear > tFar) { std::swap(tNear, tFar); }

			tMin = std::max(tMin, tNear);
			tMax = std::min(tMax, tFar);

			if (tMin > tMax) { return false; }
		}

		hitDistance = tMin;
		return true;
	}

	std::string PrettifySkyboxName(const std::string& path)
	{
		size_t lastSlash = path.find_last_of("/\\");
		std::string name = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);

		size_t lastDot = name.find_last_of('.');
		if (lastDot != std::string::npos) { name = name.substr(0, lastDot); }

		bool newWord = true;

		for (auto& character : name)
		{
			if (character == '_') { character = ' '; newWord = true; continue; }
			if (newWord) { character = static_cast<char>(std::toupper(static_cast<unsigned char>(character))); newWord = false; }
		}

		return name;
	}

	bool FuzzyMatch(const char* query, const std::string& target)
	{
		const char* q = query;

		for (char c : target)
		{
			if (*q == '\0') { return true; }

			if (std::tolower(static_cast<unsigned char>(c)) == std::tolower(static_cast<unsigned char>(*q)))
			{
				q++;
			}
		}

		return *q == '\0';
	}
}


UI::UI(int screenWidth, int screenHeight, int consoleWindowHeight, int propertiesWindowWidth, std::deque<std::string>& messages, std::vector<std::unique_ptr<Model>>& models, bool& isLit, Light& light, Grid* parentGrid, Camera& camera, Environment& environment, PostProcess& post)
	: m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),
	m_consoleWindowHeight(consoleWindowHeight),
	m_propertiesWindowWidth(propertiesWindowWidth),
	m_messages(messages),
	m_models(models),
	m_isLit(isLit),
	m_light(light),
	m_parentGrid(parentGrid),
	m_camera(camera),
	m_environment(environment),
	m_post(post)
{
	ApplyStyle(m_lightTheme);
	RefreshSkyboxList();

	m_iconsLoaded =
		m_iconMove.Load("Assets/Icons/move.png") &&
		m_iconRotate.Load("Assets/Icons/rotate.png") &&
		m_iconScale.Load("Assets/Icons/scale.png");
}

void UI::RefreshSkyboxList()
{
	m_skyboxFiles.clear();

	std::error_code errorCode;

	for (const auto& entry : std::filesystem::directory_iterator("Assets/HDRI", errorCode))
	{
		if (!entry.is_regular_file(errorCode)) { continue; }

		std::string extension = entry.path().extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char character) { return static_cast<char>(std::tolower(character)); });

		if (extension == ".hdr")
		{
			m_skyboxFiles.push_back(entry.path().generic_string());
		}
	}

	std::sort(m_skyboxFiles.begin(), m_skyboxFiles.end());
}

void UI::SetFonts(ImFont* titleFont, ImFont* headingFont)
{
	m_titleFont = titleFont;
	m_headingFont = headingFont;
}

void UI::ApplyStyle(bool lightTheme)
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowPadding = ImVec2(14.0f, 12.0f);
	style.FramePadding = ImVec2(10.0f, 6.0f);
	style.CellPadding = ImVec2(6.0f, 4.0f);
	style.ItemSpacing = ImVec2(10.0f, 8.0f);
	style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
	style.IndentSpacing = 20.0f;
	style.ScrollbarSize = 12.0f;
	style.GrabMinSize = 10.0f;

	style.WindowRounding = 8.0f;
	style.ChildRounding = 6.0f;
	style.FrameRounding = 5.0f;
	style.PopupRounding = 6.0f;
	style.ScrollbarRounding = 12.0f;
	style.GrabRounding = 12.0f;
	style.TabRounding = 6.0f;

	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.PopupBorderSize = 1.0f;

	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	const ImVec4 accent(0.259f, 0.522f, 0.957f, 1.00f);
	const ImVec4 accentHover(0.376f, 0.647f, 0.973f, 1.00f);
	const ImVec4 accentActive(0.196f, 0.427f, 0.867f, 1.00f);
	const ImVec4 accentDeep(0.125f, 0.318f, 0.702f, 1.00f);

	gLightThemeActive = lightTheme;

	ImVec4 background(0.075f, 0.078f, 0.094f, 1.00f);
	ImVec4 surface(0.114f, 0.119f, 0.145f, 1.00f);
	ImVec4 surfaceHover(0.157f, 0.163f, 0.198f, 1.00f);
	ImVec4 surfaceActive(0.196f, 0.204f, 0.247f, 1.00f);
	ImVec4 border(0.165f, 0.169f, 0.204f, 1.00f);
	ImVec4 text(0.902f, 0.902f, 0.922f, 1.00f);
	ImVec4 textDim(0.604f, 0.604f, 0.647f, 1.00f);
	ImVec4 popup(0.090f, 0.094f, 0.113f, 0.98f);
	ImVec4 titleActive(0.106f, 0.110f, 0.137f, 1.00f);

	if (lightTheme)
	{
		background = ImVec4(0.941f, 0.945f, 0.957f, 1.00f);
		surface = ImVec4(0.878f, 0.882f, 0.902f, 1.00f);
		surfaceHover = ImVec4(0.827f, 0.835f, 0.867f, 1.00f);
		surfaceActive = ImVec4(0.769f, 0.780f, 0.827f, 1.00f);
		border = ImVec4(0.796f, 0.804f, 0.835f, 1.00f);
		text = ImVec4(0.114f, 0.118f, 0.145f, 1.00f);
		textDim = ImVec4(0.427f, 0.435f, 0.482f, 1.00f);
		popup = ImVec4(0.918f, 0.922f, 0.937f, 0.98f);
		titleActive = ImVec4(0.898f, 0.902f, 0.918f, 1.00f);
	}

	ImVec4* colors = style.Colors;

	colors[ImGuiCol_Text] = text;
	colors[ImGuiCol_TextDisabled] = textDim;
	colors[ImGuiCol_WindowBg] = background;
	colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_PopupBg] = popup;
	colors[ImGuiCol_Border] = border;
	colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_FrameBg] = surface;
	colors[ImGuiCol_FrameBgHovered] = surfaceHover;
	colors[ImGuiCol_FrameBgActive] = surfaceActive;
	colors[ImGuiCol_TitleBg] = background;
	colors[ImGuiCol_TitleBgActive] = titleActive;
	colors[ImGuiCol_TitleBgCollapsed] = background;
	colors[ImGuiCol_MenuBarBg] = background;
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_ScrollbarGrab] = surfaceHover;
	colors[ImGuiCol_ScrollbarGrabHovered] = surfaceActive;
	colors[ImGuiCol_ScrollbarGrabActive] = accent;
	colors[ImGuiCol_CheckMark] = accentHover;
	colors[ImGuiCol_SliderGrab] = accent;
	colors[ImGuiCol_SliderGrabActive] = accentHover;
	colors[ImGuiCol_Button] = surfaceHover;
	colors[ImGuiCol_ButtonHovered] = accent;
	colors[ImGuiCol_ButtonActive] = accentActive;
	colors[ImGuiCol_Header] = surfaceHover;
	colors[ImGuiCol_HeaderHovered] = surfaceActive;
	colors[ImGuiCol_HeaderActive] = accent;
	colors[ImGuiCol_Separator] = border;
	colors[ImGuiCol_SeparatorHovered] = accent;
	colors[ImGuiCol_SeparatorActive] = accentHover;
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_ResizeGripHovered] = accent;
	colors[ImGuiCol_ResizeGripActive] = accentHover;
	colors[ImGuiCol_Tab] = surface;
	colors[ImGuiCol_TabHovered] = accent;
	colors[ImGuiCol_TabActive] = accentActive;
	colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
	colors[ImGuiCol_NavHighlight] = accent;

	if (lightTheme)
	{
		colors[ImGuiCol_CheckMark] = accentDeep;
		colors[ImGuiCol_SliderGrab] = accentDeep;
	}
}

bool UI::SectionHeader(const char* label)
{
	ImGui::Dummy(ImVec2(0.0f, 4.0f));

	ImVec4 headerText = gLightThemeActive ? ImVec4(0.114f, 0.118f, 0.145f, 1.0f) : kAccentHover;

	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, headerText);

	bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);

	ImGui::PopStyleColor(2);

	ImGui::Separator();

	if (open) { ImGui::Dummy(ImVec2(0.0f, 2.0f)); }

	return open;
}

void UI::Hint(const char* text)
{
	if (!ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) { return; }

	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(260.0f);
	ImGui::TextUnformatted(text);
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

bool UI::AxisDragRow(const char* label, glm::vec3& value, float speed, float min, float max, float resetValue, const char* hint)
{
	const char* axisNames[3] = { "X", "Y", "Z" };

	const ImVec4 axisColors[3] =
	{
		ImVec4(0.71f, 0.24f, 0.27f, 1.0f),
		ImVec4(0.25f, 0.52f, 0.30f, 1.0f),
		ImVec4(0.24f, 0.36f, 0.75f, 1.0f),
	};

	const ImVec4 axisHoverColors[3] =
	{
		ImVec4(0.83f, 0.32f, 0.35f, 1.0f),
		ImVec4(0.32f, 0.63f, 0.38f, 1.0f),
		ImVec4(0.32f, 0.45f, 0.87f, 1.0f),
	};

	bool changed = false;

	ImGui::PushID(label);

	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(label);

	if (hint) { Hint(hint); }

	ImGui::SameLine(88.0f);

	const float badgeWidth = 22.0f;
	const float gap = 4.0f;

	float available = ImGui::GetContentRegionAvail().x;
	float cellWidth = (available - 2.0f * gap) / 3.0f;

	for (int i = 0; i < 3; i++)
	{
		if (i > 0) { ImGui::SameLine(0.0f, gap); }

		ImGui::PushID(i);

		ImGui::PushStyleColor(ImGuiCol_Button, axisColors[i]);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, axisHoverColors[i]);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, axisColors[i]);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

		if (ImGui::Button(axisNames[i], ImVec2(badgeWidth, 0.0f)))
		{
			value[i] = resetValue;
			changed = true;
		}

		ImGui::PopStyleColor(4);

		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Reset %s to %.2f", axisNames[i], resetValue); }

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::SetNextItemWidth(cellWidth - badgeWidth);

		if (ImGui::DragFloat("##value", &value[i], speed, min, max, "%.2f")) { changed = true; }

		ImGui::PopID();
	}

	ImGui::PopID();

	return changed;
}

bool UI::SliderDrag(const char* label, float* value, float min, float max, const char* format)
{
	ImGuiID id = ImGui::GetCurrentWindow()->GetID(label);

	ImVec2 framePos = ImGui::GetCursorScreenPos();
	float frameWidth = ImGui::CalcItemWidth();

	float speed = (max - min) * 0.004f;

	bool changed = ImGui::DragFloat(label, value, speed, min, max, format, ImGuiSliderFlags_AlwaysClamp);

	if (!ImGui::TempInputIsActive(id) && max > min)
	{
		float t = (glm::clamp(*value, min, max) - min) / (max - min);

		float frameHeight = ImGui::GetFrameHeight();

		ImVec2 fillStart(framePos.x + 2.0f, framePos.y + frameHeight - 4.0f);
		ImVec2 fillEnd(framePos.x + 2.0f + (frameWidth - 4.0f) * t, framePos.y + frameHeight - 2.0f);

		if (fillEnd.x > fillStart.x)
		{
			ImGui::GetWindowDrawList()->AddRectFilled(fillStart, fillEnd, ImGui::GetColorU32(ImGuiCol_SliderGrab), 1.0f);
		}
	}

	return changed;
}

bool UI::ColorPickerRow(const char* label, float* color, bool hasAlpha)
{
	bool changed = false;

	ImGui::PushID(label);

	float width = ImGui::CalcItemWidth();

	ImGuiColorEditFlags previewFlags = hasAlpha ? ImGuiColorEditFlags_AlphaPreviewHalf : ImGuiColorEditFlags_None;

	ImVec4 preview(color[0], color[1], color[2], hasAlpha ? color[3] : 1.0f);

	if (ImGui::ColorButton("##swatch", preview, previewFlags, ImVec2(width, ImGui::GetFrameHeight())))
	{
		ImGui::OpenPopup("##picker");
	}

	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::TextUnformatted(label);

	if (ImGui::BeginPopup("##picker"))
	{
		const float pickerWidth = 210.0f;

		ImGui::TextColored(kDim, "%s", label);

		ImGui::SameLine(pickerWidth - 12.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kError);

		if (ImGui::SmallButton("x")) { ImGui::CloseCurrentPopup(); }

		ImGui::PopStyleColor(2);

		ImGui::SetNextItemWidth(pickerWidth);

		if (hasAlpha)
		{
			changed = ImGui::ColorPicker4("##colors", color, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
		}
		else
		{
			changed = ImGui::ColorPicker3("##colors", color);
		}

		ImGui::EndPopup();
	}

	ImGui::PopID();

	return changed;
}

Model* UI::CurrentModel()
{
	if (m_models.empty() || m_selectedModel < 0) { return nullptr; }

	if (m_selectedModel >= static_cast<int>(m_models.size()))
	{
		m_selectedModel = static_cast<int>(m_models.size()) - 1;
	}

	return m_models[m_selectedModel].get();
}

bool UI::ConsumeScreenshotRequest()
{
	bool requested = m_screenshotRequested;
	m_screenshotRequested = false;
	return requested;
}

void UI::PollConsoleMessages()
{
	auto message = Utility::ReadMessage();

	if (message.empty()) { return; }

	m_messages.push_front(message);

	while (m_messages.size() > 100) { m_messages.pop_back(); }
}

void UI::LoadModel(const std::string& filePath)
{
	if (filePath.empty()) { return; }

	auto newModel = std::make_unique<Model>(m_parentGrid);

	if (!newModel->Load(filePath))
	{
		return; 
	}

	newModel->GetTransform().SetIdentity();
	newModel->SetColor(glm::vec4(1.0f));

	if (newModel->HasTexturedMaterial())
	{
		newModel->IsTextured(true);
		m_isLit = true;
	}

	if (!m_models.empty())
	{
		newModel->GetTransform().SetPosition(3.0f * static_cast<float>(m_models.size()), 0.0f, 0.0f);
	}

	m_models.push_back(std::move(newModel));
	m_selectedModel = static_cast<int>(m_models.size()) - 1;

	size_t lastSlash = filePath.find_last_of("/\\");
	std::string fileName = (lastSlash == std::string::npos) ? filePath : filePath.substr(lastSlash + 1);

	Utility::AddMessage("Loaded " + fileName + " (" + FormatThousands(m_models.back()->GetTriangleCount()) + " triangles)");

	m_camera.FrameModel();
	m_easeGridHome = true;
}

void UI::RemoveModel(int index)
{
	if (index < 0 || index >= static_cast<int>(m_models.size())) { return; }

	std::string name = m_models[index]->GetDisplayName();

	m_models.erase(m_models.begin() + index);

	if (m_selectedModel >= static_cast<int>(m_models.size()))
	{
		m_selectedModel = static_cast<int>(m_models.size()) - 1;
	}

	Utility::AddMessage("Removed " + name);
}

void UI::Update(float deltaTime)
{
	m_deltaTime = deltaTime;

	PollConsoleMessages();

	auto input = Input::Instance();

	const std::string& droppedFile = input->GetDroppedFile();

	if (!droppedFile.empty())
	{
		LoadModel(droppedFile);
	}

	ImGuiIO& io = ImGui::GetIO();

	SDL_Keycode key = input->GetKeyTapped();
	bool ctrl = input->IsCtrlDown();
	bool shift = input->IsShiftDown();

	if (key == SDLK_k && ctrl)
	{
		if (m_paletteOpen) { m_paletteOpen = false; }
		else { OpenPalette(); }
	}
	else if (key == SDLK_o && ctrl)
	{
		LoadModel(FileDialog::OpenFile());
	}
	else if (key == SDLK_s && ctrl)
	{
		m_screenshotRequested = true;
	}
	else if (!m_paletteOpen && !io.WantTextInput)
	{
		if (key == SDLK_f && CurrentModel())
		{
			m_camera.FrameModel();
			m_easeGridHome = true;
		}
		else if (key == SDLK_w && !m_camera.IsNavigating())
		{
			m_gizmoOperation = 0;
		}
		else if (key == SDLK_e && !m_camera.IsNavigating())
		{
			m_gizmoOperation = 1;
		}
		else if (key == SDLK_r && !m_camera.IsNavigating())
		{
			m_gizmoOperation = 2;
		}
		else if (key == SDLK_DELETE && CurrentModel())
		{
			RemoveModel(m_selectedModel);
		}
		else if (key == SDLK_t)
		{
			m_turntable = !m_turntable;
		}
		else if (key == SDLK_SLASH && shift)
		{
			m_showShortcuts = !m_showShortcuts;
		}
		else if (key == SDLK_ESCAPE)
		{
			m_showShortcuts = false;
		}
	}

	if (m_turntable && m_parentGrid)
	{
		auto rotation = m_parentGrid->GetTransform().GetRotation();
		rotation.y += m_turntableSpeed * deltaTime;

		while (rotation.y > 360.0f) { rotation.y -= 360.0f; }

		m_parentGrid->GetTransform().SetRotation(rotation.x, rotation.y, rotation.z);
	}

	if (m_easeGridHome && m_parentGrid)
	{
		auto rotation = m_parentGrid->GetTransform().GetRotation();

		auto normalizeAngle = [](float angle)
		{
			while (angle > 180.0f) { angle -= 360.0f; }
			while (angle < -180.0f) { angle += 360.0f; }
			return angle;
		};

		rotation.x = normalizeAngle(rotation.x);
		rotation.z = normalizeAngle(rotation.z);

		float t = 1.0f - expf(-8.0f * deltaTime);

		rotation.x *= (1.0f - t);
		rotation.z *= (1.0f - t);

		bool yDone = true;

		if (!m_turntable)
		{
			rotation.y = normalizeAngle(rotation.y);
			rotation.y *= (1.0f - t);
			yDone = fabsf(rotation.y) < 0.1f;
		}

		if (fabsf(rotation.x) < 0.1f && fabsf(rotation.z) < 0.1f && yDone)
		{
			rotation.x = 0.0f;
			rotation.z = 0.0f;

			if (!m_turntable) { rotation.y = 0.0f; }

			m_easeGridHome = false;
		}

		m_parentGrid->GetTransform().SetRotation(rotation.x, rotation.y, rotation.z);
	}
}

void UI::RenderDockspace()
{
	ImGuizmo::BeginFrame();

	ImGuiID dockspaceId = ImGui::GetID("MICRODockspaceV2");

	bool buildDefaultLayout = (ImGui::DockBuilderGetNode(dockspaceId) == nullptr);

	ImGui::DockSpaceOverViewport(dockspaceId, ImGui::GetMainViewport());

	if (buildDefaultLayout)
	{
		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

		ImGuiID centerId = dockspaceId;
		ImGuiID rightId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Right, 0.30f, nullptr, &centerId);
		ImGuiID outlinerId = ImGui::DockBuilderSplitNode(rightId, ImGuiDir_Up, 0.24f, nullptr, &rightId);
		ImGuiID bottomId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Down, 0.32f, nullptr, &centerId);

		ImGui::DockBuilderDockWindow("Scene", centerId);
		ImGui::DockBuilderDockWindow("Outliner", outlinerId);
		ImGui::DockBuilderDockWindow("Properties", rightId);
		ImGui::DockBuilderDockWindow("Console", bottomId);

		ImGui::DockBuilderFinish(dockspaceId);
	}
}

void UI::RenderSceneWindow(unsigned int colorTexture)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);

	m_viewportPos = ImGui::GetCursorScreenPos();

	ImVec2 available = ImGui::GetContentRegionAvail();
	m_viewportSize = ImVec2(std::max(available.x, 64.0f), std::max(available.y, 64.0f));

	ImGui::Image((ImTextureID)(intptr_t)colorTexture, m_viewportSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	m_viewportHoveredLastFrame = m_viewportHovered;
	m_viewportHovered = ImGui::IsWindowHovered();

	RenderGizmo();
	HandleViewportPicking();

	ImGui::End();
	ImGui::PopStyleVar();
}

void UI::RenderGizmo()
{
	Model* model = CurrentModel();

	if (!model || !model->Visible())
	{
		m_gizmoBusy = false;
		return;
	}

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(m_viewportPos.x, m_viewportPos.y, m_viewportSize.x, m_viewportSize.y);

	glm::mat4 gridMatrix = m_parentGrid ? m_parentGrid->GetTransform().GetMatrix() : glm::mat4(1.0f);
	glm::mat4 matrix = gridMatrix * model->GetTransform().GetMatrix();

	ImGuizmo::OPERATION operation =
		(m_gizmoOperation == 1) ? ImGuizmo::ROTATE :
		(m_gizmoOperation == 2) ? ImGuizmo::SCALE : ImGuizmo::TRANSLATE;

	bool snapping = m_snapEnabled || ImGui::GetIO().KeyCtrl;

	float snapValues[3] = { 0.5f, 0.5f, 0.5f };

	if (operation == ImGuizmo::ROTATE) { snapValues[0] = snapValues[1] = snapValues[2] = 15.0f; }
	if (operation == ImGuizmo::SCALE) { snapValues[0] = snapValues[1] = snapValues[2] = 0.1f; }

	if (ImGuizmo::Manipulate(
		glm::value_ptr(m_camera.GetViewMatrix()), glm::value_ptr(m_camera.GetProjectionMatrix()),
		operation, ImGuizmo::LOCAL, glm::value_ptr(matrix), nullptr, snapping ? snapValues : nullptr))
	{
		glm::mat4 localMatrix = glm::inverse(gridMatrix) * matrix;

		float translation[3];
		float rotation[3];
		float scale[3];

		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localMatrix), translation, rotation, scale);

		model->GetTransform().SetPosition(translation[0], translation[1], translation[2]);
		model->GetTransform().SetRotation(rotation[0], rotation[1], rotation[2]);
		model->GetTransform().SetScale(scale[0], scale[1], scale[2]);
	}

	m_gizmoBusy = ImGuizmo::IsOver() || ImGuizmo::IsUsing();
}

void UI::HandleViewportPicking()
{
	auto input = Input::Instance();

	bool leftDown = input->IsLeftButtonClicked();

	if (input->WasLeftButtonPressed() && (m_viewportHovered || m_viewportHoveredLastFrame) && !m_gizmoBusy)
	{
		ImVec2 mouse(static_cast<float>(input->GetMousePositionX()), static_cast<float>(input->GetMousePositionY()));

		bool insideViewport =
			mouse.x >= m_viewportPos.x && mouse.x < m_viewportPos.x + m_viewportSize.x &&
			mouse.y >= m_viewportPos.y && mouse.y < m_viewportPos.y + m_viewportSize.y;

		if (insideViewport)
		{
			m_clickStartedInViewport = true;
			m_clickTravel = 0.0f;
			m_clickPos = mouse;
		}
	}

	if (leftDown)
	{
		m_clickTravel += std::fabs(static_cast<float>(input->GetMouseMotionX())) + std::fabs(static_cast<float>(input->GetMouseMotionY()));
	}

	if (!leftDown && m_leftDownLast && m_clickStartedInViewport)
	{
		if (m_clickTravel < 5.0f && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
		{
			PickModel(m_clickPos.x, m_clickPos.y);
		}

		m_clickStartedInViewport = false;
	}

	m_leftDownLast = leftDown;
}

void UI::PickModel(float mouseX, float mouseY)
{
	if (m_models.empty() || m_viewportSize.x <= 0.0f || m_viewportSize.y <= 0.0f) { return; }

	float ndcX = ((mouseX - m_viewportPos.x) / m_viewportSize.x) * 2.0f - 1.0f;
	float ndcY = 1.0f - ((mouseY - m_viewportPos.y) / m_viewportSize.y) * 2.0f;

	if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f) { return; }

	glm::mat4 inverseViewProjection = glm::inverse(m_camera.GetProjectionMatrix() * m_camera.GetViewMatrix());

	glm::vec4 nearPoint = inverseViewProjection * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
	glm::vec4 farPoint = inverseViewProjection * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

	nearPoint /= nearPoint.w;
	farPoint /= farPoint.w;

	glm::vec3 rayOrigin(nearPoint);
	glm::vec3 rayDirection = glm::normalize(glm::vec3(farPoint) - rayOrigin);

	glm::mat4 gridMatrix = m_parentGrid ? m_parentGrid->GetTransform().GetMatrix() : glm::mat4(1.0f);

	int closest = -1;
	float closestDistance = 1e30f;

	for (int i = 0; i < static_cast<int>(m_models.size()); i++)
	{
		if (!m_models[i]->Visible()) { continue; }

		glm::mat4 inverseModel = glm::inverse(gridMatrix * m_models[i]->GetTransform().GetMatrix());

		glm::vec3 localOrigin = glm::vec3(inverseModel * glm::vec4(rayOrigin, 1.0f));
		glm::vec3 localDirection = glm::vec3(inverseModel * glm::vec4(rayDirection, 0.0f));

		float hitDistance = 0.0f;

		if (RayIntersectsBox(localOrigin, localDirection, m_models[i]->GetBoundsMin(), m_models[i]->GetBoundsMax(), hitDistance))
		{
			if (hitDistance < closestDistance)
			{
				closestDistance = hitDistance;
				closest = i;
			}
		}
	}

	m_selectedModel = closest;
}

bool UI::IsViewportHovered() const
{
	return m_viewportHovered;
}

bool UI::IsGizmoBusy() const
{
	return m_gizmoBusy;
}

void UI::RenderOutlinerWindow()
{
	ImGui::Begin("Outliner", nullptr, ImGuiWindowFlags_NoCollapse);

	if (ImGui::Button("+ Add Model...", ImVec2(-1.0f, 0.0f)))
	{
		LoadModel(FileDialog::OpenFile());
	}

	Hint("Load another model into the scene. Existing models stay where they are.");

	if (m_models.empty())
	{
		ImGui::TextDisabled("Scene is empty.");
	}

	int pendingRemove = -1;

	for (int i = 0; i < static_cast<int>(m_models.size()); i++)
	{
		ImGui::PushID(i);

		ImGui::Checkbox("##visible", &m_models[i]->Visible());
		Hint("Show or hide this model.");

		ImGui::SameLine();

		float removeWidth = 26.0f;
		std::string name = m_models[i]->GetDisplayName();

		if (ImGui::Selectable(name.c_str(), m_selectedModel == i, 0, ImVec2(ImGui::GetContentRegionAvail().x - removeWidth, 0.0f)))
		{
			m_selectedModel = i;
		}

		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("%s", m_models[i]->GetSourceFile().c_str()); }

		ImGui::SameLine(0.0f, 4.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kError);

		if (ImGui::SmallButton("x"))
		{
			pendingRemove = i;
		}

		ImGui::PopStyleColor(2);

		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("Remove from scene (Del removes the selected model)"); }

		ImGui::PopID();
	}

	if (pendingRemove >= 0)
	{
		RemoveModel(pendingRemove);
	}

	ImGui::End();
}

void UI::GetViewportSize(int& width, int& height) const
{
	width = (int)m_viewportSize.x;
	height = (int)m_viewportSize.y;
}

void UI::RenderConsoleWindow()
{
	ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoCollapse);

	if (m_messages.empty())
	{
		ImGui::TextDisabled("Engine feedback appears here | Model loads, warnings, screenshots.");
	}
	else
	{
		if (ImGui::SmallButton("Clear")) { m_messages.clear(); }

		if (!m_messages.empty())
		{
			ImGui::SameLine();
			ImGui::TextDisabled("%d entries", (int)m_messages.size());
			ImGui::Dummy(ImVec2(0.0f, 2.0f));
		}
	}

	for (const auto& message : m_messages)
	{
		int category = ClassifyMessage(message);

		ImVec4 color = (category == 2) ? kError : (category == 1) ? kSuccess : ImGui::GetStyle().Colors[ImGuiCol_Text];

		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::TextWrapped("%s", message.c_str());
		ImGui::PopStyleColor();
	}

	ImGui::End();
}

void UI::RenderPropertiesWindow()
{
	ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoCollapse);

	RenderLoaderSection();
	RenderTransformSection();
	RenderMaterialSection();
	RenderCameraSection();
	RenderEnvironmentSection();
	RenderPostSection();
	RenderLightSection();

	if (SectionHeader("INTERFACE"))
	{
		if (ImGui::Checkbox("Light Theme", &m_lightTheme))
		{
			ApplyStyle(m_lightTheme);
		}

		Hint("Switch the editor between dark and light chrome. The viewport is unaffected.");

		ImGui::SameLine(0.0f, 24.0f);

		ImGui::Checkbox("Stats", &m_showStats);
		Hint("Show the FPS and geometry overlay in the viewport corner.");
	}

	ImGui::End();
}

void UI::RenderPostSection()
{
	if (!SectionHeader("RENDERING")) { return; }

	ImGui::PushItemWidth(-110.0f);

	SliderDrag("Exposure", &m_post.GetExposure(), 0.1f, 4.0f, "%.2f");
	Hint("Overall scene brightness before tonemapping - like a camera's exposure dial.");

	ImGui::PopItemWidth();

	ImGui::Checkbox("Bloom", &m_post.BloomEnabled());
	Hint("Soft glow around the brightest parts of the image.");

	if (m_post.BloomEnabled())
	{
		ImGui::PushItemWidth(-110.0f);

		SliderDrag("Strength", &m_post.GetBloomStrength(), 0.0f, 2.0f, "%.2f");
		Hint("How much the glow adds to the image.");

		SliderDrag("Threshold", &m_post.GetBloomThreshold(), 0.0f, 3.0f, "%.2f");
		Hint("Brightness a pixel needs before it starts to bloom.");

		ImGui::PopItemWidth();
	}
}

void UI::RenderEnvironmentSection()
{
	if (!m_environment.IsLoaded()) { return; }

	if (!SectionHeader("ENVIRONMENT")) { return; }

	ImGui::Checkbox("Show Skybox", &m_environment.ShowSkybox());
	Hint("Draw the HDR environment behind the scene.");

	ImGui::SameLine(0.0f, 24.0f);

	if (ImGui::Checkbox("Image Lighting", &m_environment.UseImageLighting()))
	{
		if (m_environment.UseImageLighting()) { m_isLit = true; }
	}

	Hint("Light the model with the environment itself (IBL). Metals pick up real reflections.");

	ImGui::PushItemWidth(-110.0f);
	SliderDrag("Environment", &m_environment.GetIntensity(), 0.0f, 3.0f, "%.2f");
	Hint("Brightness of the environment: affects the skybox and its lighting.");
	ImGui::PopItemWidth();

	if (!m_skyboxFiles.empty())
	{
		int currentIndex = -1;

		for (int i = 0; i < static_cast<int>(m_skyboxFiles.size()); i++)
		{
			if (m_skyboxFiles[i] == m_environment.GetCurrentPath()) { currentIndex = i; break; }
		}

		std::string currentName = (currentIndex >= 0) ? PrettifySkyboxName(m_skyboxFiles[currentIndex]) : PrettifySkyboxName(m_environment.GetCurrentPath());

		ImGui::PushItemWidth(-110.0f);

		if (ImGui::BeginCombo("Skybox", currentName.c_str()))
		{
			for (int i = 0; i < static_cast<int>(m_skyboxFiles.size()); i++)
			{
				bool selected = (i == currentIndex);

				if (ImGui::Selectable(PrettifySkyboxName(m_skyboxFiles[i]).c_str(), selected) && !selected)
				{
					m_environment.Load(m_skyboxFiles[i]);
				}

				if (selected) { ImGui::SetItemDefaultFocus(); }
			}

			ImGui::EndCombo();
		}

		Hint("Swap the HDR environment. Drop more .hdr files into Assets/HDRI to extend this list. Rebaking takes about a second.");

		ImGui::PopItemWidth();
	}
}

void UI::RenderLoaderSection()
{
	if (!SectionHeader("MODEL")) { return; }

	Model* model = CurrentModel();

	if (model)
	{
		const std::string& sourceFile = model->GetSourceFile();

		size_t lastSlash = sourceFile.find_last_of("/\\");
		std::string fileName = (lastSlash == std::string::npos) ? sourceFile : sourceFile.substr(lastSlash + 1);

		ImGui::TextColored(kDim, "File");
		ImGui::SameLine(88.0f);
		ImGui::TextUnformatted(fileName.c_str());

		if (ImGui::IsItemHovered()) { ImGui::SetTooltip("%s", sourceFile.c_str()); }

		ImGui::TextColored(kDim, "Size");
		ImGui::SameLine(88.0f);
		ImGui::Text("%s tris  |  %s verts  |  %d %s",
			FormatThousands(model->GetTriangleCount()).c_str(),
			FormatThousands(model->GetVertexCount()).c_str(),
			model->GetMeshCount(),
			model->GetMeshCount() == 1 ? "mesh" : "meshes");
	}
	else
	{
		ImGui::TextDisabled("No model loaded yet.");
	}

	if (ImGui::Button("Load New Model...", ImVec2(-1.0f, 34.0f)))
	{
		LoadModel(FileDialog::OpenFile());
	}

	Hint("Browse for a model - or simply drag & drop a file anywhere in this window. (Ctrl+O)");

	if (ImGui::Button("Save Screenshot", ImVec2(-1.0f, 28.0f)))
	{
		m_screenshotRequested = true;
	}

	Hint("Saves a PNG of the viewport into the Screenshots folder, next to the executable. (Ctrl+S)");
}

void UI::RenderTransformSection()
{
	Model* model = CurrentModel();

	if (!model) { return; }

	if (!SectionHeader("TRANSFORM")) { return; }

	auto position = model->GetTransform().GetPosition();

	if (AxisDragRow("Position", position, 0.02f, -50.0f, 50.0f, 0.0f, "Drag a field to move the model. Click X, Y or Z to reset that axis."))
	{
		model->GetTransform().SetPosition(position.x, position.y, position.z);
	}

	auto rotation = model->GetTransform().GetRotation();

	if (AxisDragRow("Rotation", rotation, 0.5f, 0.0f, 0.0f, 0.0f, "Drag a field to rotate the model in degrees. Click X, Y or Z to reset that axis."))
	{
		model->GetTransform().SetRotation(rotation.x, rotation.y, rotation.z);
	}

	auto scale = model->GetTransform().GetScale();

	if (AxisDragRow("Scale", scale, 0.02f, 0.05f, 20.0f, 1.0f, "Drag a field to scale the model. Click X, Y or Z to reset that axis."))
	{
		model->GetTransform().SetScale(scale.x, scale.y, scale.z);
	}
}

void UI::RenderMaterialSection()
{
	Model* model = CurrentModel();

	if (!model) { return; }

	if (!SectionHeader("MATERIAL")) { return; }

	const int chipsPerRow = 4;
	const float gap = 6.0f;

	float available = ImGui::GetContentRegionAvail().x;
	float chipWidth = (available - gap * (chipsPerRow - 1)) / chipsPerRow;

	bool originalSelected = model->GetMaterialPresetName().empty();

	{
		ImGui::PushStyleColor(ImGuiCol_Border, kAccent);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, originalSelected ? 2.0f : 0.0f);

		if (ImGui::Button("Original", ImVec2(chipWidth, 26.0f)))
		{
			model->ClearMaterialPreset();
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		Hint("Restore the materials from the model file.");
	}

	int chipIndex = 1;

	for (const auto& preset : kMaterialPresets)
	{
		if (chipIndex % chipsPerRow != 0) { ImGui::SameLine(0.0f, gap); }

		bool selected = model->GetMaterialPresetName() == preset.name;

		float luminance = 0.299f * preset.tint.r + 0.587f * preset.tint.g + 0.114f * preset.tint.b;
		ImVec4 textColor = (luminance > 0.55f) ? ImVec4(0.08f, 0.08f, 0.10f, 1.0f) : ImVec4(0.95f, 0.95f, 0.95f, 1.0f);

		ImVec4 chipColor(preset.tint.r, preset.tint.g, preset.tint.b, 1.0f);
		ImVec4 chipHover(preset.tint.r * 1.1f, preset.tint.g * 1.1f, preset.tint.b * 1.1f, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, chipColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, chipHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, chipColor);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.95f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, selected ? 2.0f : 0.0f);

		if (ImGui::Button(preset.name, ImVec2(chipWidth, 26.0f)))
		{
			model->ApplyMaterialPreset(preset.name, preset.tint, preset.roughness, preset.metallic);
			m_isLit = true; 
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(5);

		Hint("Non-destructive PBR preset. \"Original\" brings the file's own materials back.");

		chipIndex++;
	}
}

void UI::RenderCameraSection()
{
	if (!SectionHeader("CAMERA")) { return; }

	struct PresetButton
	{
		const char* label;
		Camera::Preset preset;
	};

	const PresetButton presetButtons[] =
	{
		{ "Persp", Camera::Preset::Perspective },
		{ "Front", Camera::Preset::Front },
		{ "Top",   Camera::Preset::Top },
		{ "Back",  Camera::Preset::Back },
		{ "Left",  Camera::Preset::Left },
		{ "Right", Camera::Preset::Right },
	};

	const int buttonsPerRow = 3;
	const float gap = 6.0f;

	float available = ImGui::GetContentRegionAvail().x;
	float buttonWidth = (available - gap * (buttonsPerRow - 1)) / buttonsPerRow;

	for (int i = 0; i < 6; i++)
	{
		if (i % buttonsPerRow != 0) { ImGui::SameLine(0.0f, gap); }

		if (ImGui::Button(presetButtons[i].label, ImVec2(buttonWidth, 0.0f)))
		{
			m_camera.ApplyPreset(presetButtons[i].preset);
			m_easeGridHome = true;
		}

		Hint("Smoothly fly the camera to this view. The scene also eases back upright.");
	}

	if (CurrentModel())
	{
		if (ImGui::Button("Frame Model", ImVec2(-1.0f, 0.0f)))
		{
			m_camera.FrameModel();
			m_easeGridHome = true;
		}

		Hint("Fly to a nicely framed hero shot of the model. (F)");
	}

	ImGui::Checkbox("Turntable", &m_turntable);
	Hint("Auto-orbit the scene - perfect for showing off a model or recording a clip. (T)");

	if (m_turntable)
	{
		ImGui::SameLine(0.0f, 16.0f);
		ImGui::SetNextItemWidth(-74.0f);
		SliderDrag("Speed", &m_turntableSpeed, 5.0f, 90.0f, "%.0f\xC2\xB0/s");
	}
}

void UI::RenderLightSection()
{
	if (!SectionHeader("LIGHTING")) { return; }

	ImGui::Checkbox("Enable Lighting", &m_isLit);
	Hint("Toggle the physically-based pipeline: Cook-Torrance PBR, normal maps and soft shadows.");

	Model* model = CurrentModel();

	if (m_isLit)
	{
		if (model)
		{
			ImGui::SameLine(0.0f, 24.0f);

			bool isTextured = model->IsTextured();

			if (ImGui::Checkbox("Textures", &isTextured))
			{
				model->IsTextured(isTextured);
			}

			Hint("Toggle the model's texture maps.");
		}

		auto lightPosition = m_light.GetTransform().GetPosition();

		if (AxisDragRow("Light", lightPosition, 0.05f, -20.0f, 20.0f, 4.0f, "Position of the point light. Watch the shadows move with it."))
		{
			m_light.GetTransform().SetPosition(lightPosition.x, lightPosition.y, lightPosition.z);
		}

		ImGui::PushItemWidth(-110.0f);

		ColorPickerRow("Light Color", &m_light.GetDiffuse().x, false);
		Hint("Color of the light source.");
		m_light.GetSpecular() = m_light.GetDiffuse();

		SliderDrag("Intensity", &m_light.GetIntensity(), 0.0f, 10.0f, "%.2f");
		Hint("Brightness of the light.");

		if (SliderDrag("Ambient", &m_light.GetAmbient().x, 0.0f, 1.0f, "%.2f"))
		{
			m_light.GetAmbient() = glm::vec3(m_light.GetAmbient().x);
		}

		Hint("Base fill light that keeps shadowed areas from going pitch black.");

		SliderDrag("Falloff", &m_light.GetQuadraticAttenuation(), 0.0f, 0.2f, "%.3f");
		Hint("How quickly the light fades with distance.");

		ImGui::PopItemWidth();
	}
	else if (model)
	{
		ImGui::PushItemWidth(-110.0f);

		auto color = model->GetColor();

		if (ColorPickerRow("Flat Color", &color.x, true))
		{
			model->SetColor(color);
		}

		Hint("Tint of the unlit model.");

		ImGui::PopItemWidth();
	}
}

void UI::RenderOverlays()
{
	RenderStatsOverlay();
	RenderGizmoToolbar();
	RenderEmptyState();
	RenderShortcutSheet();
	RenderCommandPalette();
}

void UI::RenderGizmoToolbar()
{
	if (m_models.empty()) { return; }

	float top = m_viewportPos.y + 12.0f + (m_showStats ? m_statsHeight + 8.0f : 0.0f);

	ImGui::SetNextWindowPos(ImVec2(m_viewportPos.x + 12.0f, top), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.68f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 3.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

	ImGui::Begin("##gizmobar", nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);

	struct ToolButton
	{
		const char* id;
		const char* fallbackLabel;
		const Texture* icon;
		const char* hint;
		int operation;
	};

	const ToolButton tools[] =
	{
		{ "##move",   "M", &m_iconMove,   "Move (W)",   0 },
		{ "##rotate", "R", &m_iconRotate, "Rotate (E)", 1 },
		{ "##scale",  "S", &m_iconScale,  "Scale (R)",  2 },
	};

	const ImVec2 iconSize(16.0f, 16.0f);
	const ImVec4 transparent(0.0f, 0.0f, 0.0f, 0.0f);

	for (const auto& tool : tools)
	{
		bool active = (m_gizmoOperation == tool.operation);

		ImGui::PushStyleColor(ImGuiCol_Button, active ? kAccent : transparent);

		bool pressed = false;

		if (m_iconsLoaded)
		{
			ImVec4 tint = active ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImGui::GetStyle().Colors[ImGuiCol_Text];

			pressed = ImGui::ImageButton(tool.id, (ImTextureID)(intptr_t)tool.icon->GetID(), iconSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), transparent, tint);
		}
		else
		{
			pressed = ImGui::Button(tool.fallbackLabel, ImVec2(iconSize.x + 8.0f, iconSize.y + 8.0f));
		}

		ImGui::PopStyleColor();

		if (pressed) { m_gizmoOperation = tool.operation; }

		Hint(tool.hint);
	}

	ImGui::Separator();

	{
		bool active = m_snapEnabled;

		ImGui::PushStyleColor(ImGuiCol_Button, active ? kAccent : transparent);
		ImGui::PushStyleColor(ImGuiCol_Text, active ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImGui::GetStyle().Colors[ImGuiCol_Text]);

		if (ImGui::Button("#", ImVec2(iconSize.x + 8.0f, iconSize.y + 8.0f)))
		{
			m_snapEnabled = !m_snapEnabled;
		}

		ImGui::PopStyleColor(2);

		Hint("Snap to grid: 0.5 units, 15 degrees, 0.1 scale. Hold Ctrl to snap temporarily.");
	}

	ImGui::End();

	ImGui::PopStyleVar(3);
}

void UI::RenderStatsOverlay()
{
	if (!m_showStats) { return; }

	ImGuiIO& io = ImGui::GetIO();

	ImGui::SetNextWindowPos(ImVec2(m_viewportPos.x + 12.0f, m_viewportPos.y + 12.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.68f);

	ImGui::Begin("##stats", nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);

	float fps = io.Framerate;
	ImVec4 fpsColor = (fps >= 59.0f) ? kSuccess : (fps >= 29.0f) ? kWarning : kError;

	ImGui::TextColored(fpsColor, "%.0f", fps);
	ImGui::SameLine(0.0f, 5.0f);
	ImGui::TextColored(kDim, "FPS");
	ImGui::SameLine(0.0f, 12.0f);
	ImGui::TextColored(kDim, "%.2f ms", 1000.0f / (fps > 0.0f ? fps : 1.0f));

	if (!m_models.empty())
	{
		int totalTriangles = 0;
		int totalVertices = 0;
		int totalMeshes = 0;
		int visibleModels = 0;

		for (const auto& model : m_models)
		{
			if (!model->Visible()) { continue; }

			totalTriangles += model->GetTriangleCount();
			totalVertices += model->GetVertexCount();
			totalMeshes += model->GetMeshCount();
			visibleModels++;
		}

		ImGui::Separator();

		ImGui::TextColored(kDim, "Triangles");
		ImGui::SameLine(86.0f);
		ImGui::Text("%s", FormatThousands(totalTriangles).c_str());

		ImGui::TextColored(kDim, "Vertices");
		ImGui::SameLine(86.0f);
		ImGui::Text("%s", FormatThousands(totalVertices).c_str());

		ImGui::TextColored(kDim, "Meshes");
		ImGui::SameLine(86.0f);
		ImGui::Text("%d", totalMeshes);

		if (m_models.size() > 1)
		{
			ImGui::TextColored(kDim, "Models");
			ImGui::SameLine(86.0f);
			ImGui::Text("%d / %d", visibleModels, static_cast<int>(m_models.size()));
		}
	}

	m_statsHeight = ImGui::GetWindowSize().y;

	ImGui::End();
}

void UI::RenderEmptyState()
{
	float target = m_models.empty() ? 1.0f : 0.0f;
	m_emptyStateAlpha += (target - m_emptyStateAlpha) * (1.0f - expf(-5.0f * m_deltaTime));

	if (m_emptyStateAlpha < 0.02f) { return; }

	ImVec2 center(m_viewportPos.x + m_viewportSize.x * 0.5f, m_viewportPos.y + m_viewportSize.y * 0.47f);

	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(520.0f, 0.0f));

	ImVec4 cardBackground(0.075f, 0.078f, 0.094f, 0.92f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28.0f, 30.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_emptyStateAlpha);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, cardBackground);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.259f, 0.522f, 0.957f, 0.28f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.902f, 0.902f, 0.922f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.157f, 0.163f, 0.198f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kAccent);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.196f, 0.427f, 0.867f, 1.0f));

	ImGuiWindowFlags cardFlags =
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing;

	if (target < 0.5f) { cardFlags |= ImGuiWindowFlags_NoInputs; }

	ImGui::Begin("##emptystate", nullptr, cardFlags);

	if (m_titleFont) { ImGui::PushFont(m_titleFont, 34.0f); }
	CenteredText("M.I.C.R.O", kAccent);
	if (m_titleFont) { ImGui::PopFont(); }

	CenteredText("G R A P H I C S   E N G I N E", kDim);

	ImGui::Dummy(ImVec2(0.0f, 14.0f));

	float pulse = 0.70f + 0.22f * sinf((float)ImGui::GetTime() * 2.2f);
	CenteredText("Drop a 3D model anywhere in this window", ImVec4(0.90f, 0.90f, 0.92f, pulse));

	CenteredText("OBJ  FBX  GLTF  GLB  DAE  STL  PLY  3DS", kDim);

	ImGui::Dummy(ImVec2(0.0f, 16.0f));

	float buttonWidth = 200.0f;
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) * 0.5f);

	if (ImGui::Button("Browse Files...", ImVec2(buttonWidth, 36.0f)))
	{
		LoadModel(FileDialog::OpenFile());
	}

	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	CenteredText("Drag orbits  |  RMB looks + WASD flies  |  Ctrl+K commands", kDim);

	ImGui::End();

	ImGui::PopStyleColor(6);
	ImGui::PopStyleVar(3);
}

void UI::RenderShortcutSheet()
{
	if (!m_showShortcuts) { return; }

	ImVec2 center(m_viewportPos.x + m_viewportSize.x * 0.5f, m_viewportPos.y + m_viewportSize.y * 0.5f);

	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(380.0f, 0.0f));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 20.0f));

	ImGui::Begin("##shortcuts", nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);

	if (m_headingFont) { ImGui::PushFont(m_headingFont, 20.0f); }
	CenteredText("Keyboard Shortcuts", ImVec4(0.902f, 0.902f, 0.922f, 1.0f));
	if (m_headingFont) { ImGui::PopFont(); }

	ImGui::Dummy(ImVec2(0.0f, 8.0f));

	auto row = [](const char* keys, const char* description)
	{
		ImGui::TextColored(kAccent, "%s", keys);
		ImGui::SameLine(110.0f);
		ImGui::TextUnformatted(description);
	};

	row("Ctrl+K", "Command palette");
	row("Ctrl+O", "Load a model");
	row("Ctrl+S", "Save a screenshot");
	row("F", "Frame the model");
	row("T", "Toggle turntable");
	row("W / E / R", "Gizmo: move / rotate / scale");
	row("Ctrl (hold)", "Snap gizmo to grid");
	row("Click", "Select a model");
	row("Del", "Remove the selected model");
	row("?", "This cheat sheet");
	row("Esc", "Close overlays");

	ImGui::Separator();

	row("RMB drag", "Look around");
	row("RMB+WASD", "Fly (Q/E down/up, Shift = fast)");
	row("LMB drag", "Orbit the model");
	row("MMB drag", "Pan");
	row("Scroll", "Zoom (fly speed while RMB)");

	ImGui::Dummy(ImVec2(0.0f, 8.0f));

	float buttonWidth = 120.0f;
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - buttonWidth) * 0.5f);

	if (ImGui::Button("Close", ImVec2(buttonWidth, 0.0f)))
	{
		m_showShortcuts = false;
	}

	ImGui::End();

	ImGui::PopStyleVar(2);
}

void UI::OpenPalette()
{
	m_paletteOpen = true;
	m_paletteJustOpened = true;
	m_paletteQuery[0] = '\0';
	m_paletteSelection = 0;

	BuildPaletteActions();
}

void UI::BuildPaletteActions()
{
	m_paletteActions.clear();

	bool hasModel = CurrentModel() != nullptr;

	auto add = [this](const char* label, const char* shortcut, std::function<void()> run)
	{
		m_paletteActions.push_back({ label, shortcut, std::move(run) });
	};

	add("Load Model...", "Ctrl+O", [this] { LoadModel(FileDialog::OpenFile()); });
	add("Save Screenshot", "Ctrl+S", [this] { m_screenshotRequested = true; });

	if (hasModel)
	{
		add("Frame Model", "F", [this] { m_camera.FrameModel(); m_easeGridHome = true; });

		add("Reset Model Transform", "", [this]
		{
			Model* model = CurrentModel();

			if (model) { model->GetTransform().SetIdentity(); }
		});

		add("Toggle Textures", "", [this]
		{
			Model* model = CurrentModel();

			if (model) { model->IsTextured(!model->IsTextured()); }
		});
	}

	if (hasModel)
	{
		add("Remove Selected Model", "Del", [this] { RemoveModel(m_selectedModel); });

		add("Gizmo: Move", "W", [this] { m_gizmoOperation = 0; });
		add("Gizmo: Rotate", "E", [this] { m_gizmoOperation = 1; });
		add("Gizmo: Scale", "R", [this] { m_gizmoOperation = 2; });
		add("Toggle Snap to Grid", "", [this] { m_snapEnabled = !m_snapEnabled; });
	}

	add("Toggle Turntable", "T", [this] { m_turntable = !m_turntable; });

	add("Toggle Lighting", "", [this] { m_isLit = !m_isLit; });
	add("Toggle Stats Overlay", "", [this] { m_showStats = !m_showStats; });
	add("Toggle Light Theme", "", [this] { m_lightTheme = !m_lightTheme; ApplyStyle(m_lightTheme); });

	add("Toggle Bloom", "", [this] { m_post.BloomEnabled() = !m_post.BloomEnabled(); });

	if (m_environment.IsLoaded())
	{
		add("Toggle Skybox", "", [this] { m_environment.ShowSkybox() = !m_environment.ShowSkybox(); });
		add("Toggle Image Lighting", "", [this]
		{
			m_environment.UseImageLighting() = !m_environment.UseImageLighting();

			if (m_environment.UseImageLighting()) { m_isLit = true; }
		});
	}

	struct PresetEntry
	{
		const char* label;
		Camera::Preset preset;
	};

	const PresetEntry presetEntries[] =
	{
		{ "Camera: Perspective", Camera::Preset::Perspective },
		{ "Camera: Front", Camera::Preset::Front },
		{ "Camera: Back", Camera::Preset::Back },
		{ "Camera: Left", Camera::Preset::Left },
		{ "Camera: Right", Camera::Preset::Right },
		{ "Camera: Top", Camera::Preset::Top },
	};

	for (const auto& entry : presetEntries)
	{
		Camera::Preset preset = entry.preset;
		add(entry.label, "", [this, preset] { m_camera.ApplyPreset(preset); m_easeGridHome = true; });
	}

	if (hasModel)
	{
		for (const auto& preset : kMaterialPresets)
		{
			std::string label = std::string("Material: ") + preset.name;
			MaterialPresetInfo info = preset;

			m_paletteActions.push_back({ label, "", [this, info]
			{
				Model* model = CurrentModel();

				if (model)
				{
					model->ApplyMaterialPreset(info.name, info.tint, info.roughness, info.metallic);
					m_isLit = true;
				}
			} });
		}

		add("Material: Original", "", [this]
		{
			Model* model = CurrentModel();

			if (model) { model->ClearMaterialPreset(); }
		});
	}

	add("Keyboard Shortcuts", "?", [this] { m_showShortcuts = true; });
	add("Clear Console", "", [this] { m_messages.clear(); });
}

void UI::RenderCommandPalette()
{
	if (!m_paletteOpen) { return; }

	ImGuiIO& io = ImGui::GetIO();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(io.DisplaySize);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.45f));

	ImGui::Begin("##palettedim", nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoFocusOnAppearing);

	bool dimClicked = ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0);

	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->GetCenter().x, 110.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(560.0f, 0.0f), ImVec2(560.0f, 520.0f));
	ImGui::SetNextWindowFocus();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 12.0f));

	ImGui::Begin("##palette", nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove);

	if (m_paletteJustOpened) { ImGui::SetKeyboardFocusHere(); }

	ImGui::SetNextItemWidth(-FLT_MIN);

	if (ImGui::InputTextWithHint("##palettequery", "Type a command...", m_paletteQuery, sizeof(m_paletteQuery)))
	{
		m_paletteSelection = 0;
	}

	std::vector<int> visible;

	for (int i = 0; i < (int)m_paletteActions.size(); i++)
	{
		if (FuzzyMatch(m_paletteQuery, m_paletteActions[i].label))
		{
			visible.push_back(i);
		}
	}

	if (m_paletteSelection >= (int)visible.size()) { m_paletteSelection = (int)visible.size() - 1; }
	if (m_paletteSelection < 0) { m_paletteSelection = 0; }

	bool navigated = false;

	if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && !visible.empty())
	{
		m_paletteSelection = (m_paletteSelection + 1) % (int)visible.size();
		navigated = true;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && !visible.empty())
	{
		m_paletteSelection = (m_paletteSelection + (int)visible.size() - 1) % (int)visible.size();
		navigated = true;
	}

	bool confirmed = ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter);
	bool cancelled = ImGui::IsKeyPressed(ImGuiKey_Escape);

	int chosen = -1;

	ImGui::Separator();

	float rowHeight = ImGui::GetTextLineHeightWithSpacing() + 6.0f;
	float listHeight = std::min(rowHeight * (float)std::max((int)visible.size(), 1) + 6.0f, 340.0f);

	ImGui::BeginChild("##palettelist", ImVec2(0.0f, listHeight), false);

	if (visible.empty())
	{
		ImGui::TextDisabled("No matching commands");
	}

	for (int row = 0; row < (int)visible.size(); row++)
	{
		const PaletteAction& action = m_paletteActions[visible[row]];

		bool selected = (row == m_paletteSelection);

		if (ImGui::Selectable(action.label.c_str(), selected, 0, ImVec2(0.0f, rowHeight - 6.0f)))
		{
			chosen = visible[row];
		}

		if (ImGui::IsItemHovered() && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f)) { m_paletteSelection = row; }

		if (selected && navigated) { ImGui::SetScrollHereY(); }

		if (!action.shortcut.empty())
		{
			ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(action.shortcut.c_str()).x - 18.0f);
			ImGui::TextDisabled("%s", action.shortcut.c_str());
		}
	}

	ImGui::EndChild();

	ImGui::Separator();
	ImGui::TextDisabled("Up / Down to navigate   |   Enter to run   |   Esc to close");

	ImGui::End();

	ImGui::PopStyleVar(2);

	if (confirmed && chosen < 0 && !visible.empty())
	{
		chosen = visible[m_paletteSelection];
	}

	if (chosen >= 0)
	{
		auto run = m_paletteActions[chosen].run;
		m_paletteOpen = false;
		run();
	}
	else if (cancelled || dimClicked)
	{
		m_paletteOpen = false;
	}

	m_paletteJustOpened = false;
}
