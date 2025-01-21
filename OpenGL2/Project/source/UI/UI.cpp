#include <Utility/FileDialog.h>
#include <Grid/Grid.h>
#include <UI/UI.h>


UI::UI(int screenWidth, int screenHeight, int consoleWindowHeight, int propertiesWindowWidth, std::deque<std::string>& messages, std::vector<std::unique_ptr<Model>>& models, bool& isLit, Grid* parentGrid)
    : m_screenWidth(screenWidth),
    m_screenHeight(screenHeight),
    m_consoleWindowHeight(consoleWindowHeight),
    m_propertiesWindowWidth(propertiesWindowWidth),
    m_messages(messages),
    m_models(models),
    m_isLit(isLit),
    m_parentGrid(parentGrid)
{
}

void UI::RenderConsoleWindow()
{
    ImGui::Begin("Console", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);


    auto windowPosition = ImVec2(0, (float)m_screenHeight - (float)m_consoleWindowHeight);
    auto windowSize = ImVec2((float)m_screenWidth - (float)m_propertiesWindowWidth, (float)m_consoleWindowHeight);


    ImGui::SetWindowPos("Console", windowPosition);
    ImGui::SetWindowSize("Console", windowSize);

    auto message = Utility::ReadMessage();

    if (!message.empty())
    {
        m_messages.push_front(message);
    }

    for (const auto& message : m_messages)
    {
        ImGui::Text(message.c_str());
    }

    ImGui::End();
}

void UI::RenderPropertiesWindow()
{
    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);


    auto windowPosition = ImVec2((float)m_screenWidth - (float)m_propertiesWindowWidth, 0.0f);
    auto windowSize = ImVec2((float)m_propertiesWindowWidth, (float)m_screenHeight);


    ImGui::SetWindowPos("Properties", windowPosition);
    ImGui::SetWindowSize("Properties", windowSize);

    ImGui::Separator();
    ImGui::Text("TRANSFORM");
    ImGui::Separator();

    auto position = m_models[0]->GetTransform().GetPosition();
    ImGui::SliderFloat3("Position", &position.x, -10.0f, 10.0f, "%.2f");
    m_models[0]->GetTransform().SetPosition(position.x, position.y, position.z);

    auto rotation = m_models[0]->GetTransform().GetRotation();
    ImGui::SliderFloat3("Rotation", &rotation.x, 0.0f, 360.0f, "%.2f");
    m_models[0]->GetTransform().SetRotation(rotation.x, rotation.y, rotation.z);

    auto scale = m_models[0]->GetTransform().GetScale();
    ImGui::SliderFloat3("Scale", &scale.x, 0.1f, 10.0f, "%.2f");
    m_models[0]->GetTransform().SetScale(scale.x, scale.y, scale.z);

    ImGui::Text(" ");

    ImGui::Separator();
    ImGui::Text("SCENE LIGHT & TEXTURE");
    ImGui::Separator();

    ImGui::Checkbox("Toggle Light", &m_isLit);

    auto isTextured = m_models[0]->IsTextured();

    if (m_isLit)
    {
        ImGui::Checkbox("Toggle Texture", &isTextured);
        m_models[0]->IsTextured(isTextured);
    }

    ImGui::Separator();

    ImGui::Text(" ");

    ImGui::Separator();
    if (!m_isLit) { ImGui::Text("COLOR EDITOR"); }
    if (m_isLit) { ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.35f), "COLOR EDITOR - Light is Active"); }
    ImGui::Separator();

    auto color = m_models[0]->GetColor();
    if (!m_isLit)
    {
        ImGui::ColorEdit4("Color", &color.x);
        m_models[0]->SetColor(color);
    }

    ImGui::Text("");

    ImGui::Separator();
    ImGui::Text("MODEL LOADER");
    ImGui::Separator();

    if (ImGui::Button("Load New Model"))
    {
        std::string filePath = FileDialog::OpenFile();

        if (!filePath.empty())
        {
            m_models[0] = std::make_unique<Model>(m_parentGrid);

            bool loaded = m_models[0]->Load(filePath);

            if (!loaded)
            {
                Utility::AddMessage("Failed to load new model from: " + filePath);
            }
            else
            {
                Utility::AddMessage("Successfully loaded: " + filePath);

                m_models[0]->GetTransform().SetIdentity();
                m_models[0]->SetColor(glm::vec4(1.0f));
            }
        }
    }

    ImGui::End();
}
