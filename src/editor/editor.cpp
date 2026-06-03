#include "editor.h"
#include "engine/gui_utilities.h"
#include "engine/resourcemanager.h"
#include "utils/logging.h"
#include "themes.h"
#include "filedialog.h"
#include <filesystem>
#include <rlImGui.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#define FILE_PROJECT_FILTER "Project file:mgd"
#define FILE_PROJECT_FILTER_BIN "Project Binary:bin"
#define FILE_PICTURE_FILTER "Picture:png,jpg,jpeg"
#define FILE_SOUND_FILTER "Sound file:mp3"

namespace fs = std::filesystem;

Editor::~Editor(){
  m_wm.clear();
  m_resManager.UnloadAll();
}

void Editor::run() {
  if (!init())
    throw std::runtime_error("[Editor::run] Unable to initialize editor!");
  if(!initImgui())
    throw std::runtime_error("[Editor::run] Unable to initialize ImGui!");
  loop();
}

void Editor::loop() {
  while (!WindowShouldClose() && !m_close) {
    m_wm.Update();
    BeginDrawing();
    ClearBackground(BLACK);
    rlImGuiBegin();
    taskbar();
    rlImGuiEnd();
    m_wm.Draw();
    EndDrawing();
  }
}

bool Editor::init() {
  if (!initResourcemanager())
    return false;
  return true;
}

bool Editor::initResourcemanager() {
  if (!m_resManager.LoadTexture("button", "textures/button.png"))
    return false;
  return true;
}

bool Editor::initImgui() {
  rlImGuiSetup(false);
	SetTheme(m_imguiSettings.theme);
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	if (fs::exists("fonts/JetBrainsMonoNerdFont-Bold.ttf")) {
		ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMonoNerdFont-Bold.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
		if (font)
			io.FontDefault = font;
		else
			logging::logwarning("[Editor::initImgui] Font could not be loaded: %s", "fonts/JetBrainsMonoNerdFont-Bold.ttf");
	}
	else {
		logging::logwarning("[Editor::initImgui] Font file does not exist: %s", "fonts/JetBrainsMonoNerdFont-Bold.ttf");
	}
  return true;
}

void Editor::taskbar() {
  if(ImGui::BeginMainMenuBar()){
    // File handling
    if(ImGui::BeginMenu("File")){
      if(ImGui::MenuItem("New Project")){
        logging::loginfo("Selected folder: %s", OpenDirectoryDialog().c_str());
      }
      if(ImGui::MenuItem("Load Project")){
        logging::loginfo("Selected Project: %s", OpenFileDialog({FILE_PROJECT_FILTER, FILE_PROJECT_FILTER_BIN}).c_str());
      }
      if(ImGui::MenuItem("Save as")){
        logging::loginfo("Saving as: %s", SaveFileDialog({FILE_PROJECT_FILTER, FILE_PROJECT_FILTER_BIN}).c_str());
      }
      if(ImGui::MenuItem("Exit"))
        m_close = true;
      ImGui::EndMenu();
    }

    // Theme selection
    if(ImGui::BeginMenu("Theme")){
      if(ImGui::MenuItem("Default", nullptr, m_imguiSettings.theme == 0)){
        m_imguiSettings.theme = Themes::DEFAULT;
        SetTheme(m_imguiSettings.theme);
      }
      ImGui::SeparatorText("Light themes");
      if(ImGui::MenuItem("Gold Light", nullptr, m_imguiSettings.theme == Themes::GOLD_LIGHT)){
        m_imguiSettings.theme = Themes::GOLD_LIGHT;
        SetTheme(m_imguiSettings.theme);
      }
      if(ImGui::MenuItem("Purple Light", nullptr, m_imguiSettings.theme == Themes::PURPLE_LIGHT)){
        m_imguiSettings.theme = Themes::PURPLE_LIGHT;
        SetTheme(m_imguiSettings.theme);
      }
      if(ImGui::MenuItem("Girly Pink", nullptr, m_imguiSettings.theme == Themes::GIRLY_PINK)){
        m_imguiSettings.theme = Themes::GIRLY_PINK;
        SetTheme(m_imguiSettings.theme);
      }
      if(ImGui::MenuItem("Noctua Light", nullptr, m_imguiSettings.theme == Themes::NOCTUA_LIGHT)){
        m_imguiSettings.theme = Themes::NOCTUA_LIGHT;
        SetTheme(m_imguiSettings.theme);
      }
      if(ImGui::MenuItem("RosePine Light", nullptr, m_imguiSettings.theme == Themes::ROSEPINE_LIGHT)){
        m_imguiSettings.theme = Themes::ROSEPINE_LIGHT;
        SetTheme(m_imguiSettings.theme);
      }
      ImGui::SeparatorText("Dark themes");
      if(ImGui::MenuItem("Gold Dark", nullptr, m_imguiSettings.theme == Themes::GOLD_DARK)){
        m_imguiSettings.theme = Themes::GOLD_DARK;
        SetTheme(m_imguiSettings.theme);
      }
      if(ImGui::MenuItem("Purple Dark", nullptr, m_imguiSettings.theme == Themes::PURPLE_DARK)){
        m_imguiSettings.theme = Themes::PURPLE_DARK;
        SetTheme(m_imguiSettings.theme);
      }
      if(ImGui::MenuItem("Noctua Dark", nullptr, m_imguiSettings.theme == Themes::NOCTUA_DARK)){
        m_imguiSettings.theme = Themes::NOCTUA_DARK;
        SetTheme(m_imguiSettings.theme);
      }
      if(ImGui::MenuItem("RosePine Dark", nullptr, m_imguiSettings.theme == Themes::ROSEPINE_DARK)){
        m_imguiSettings.theme = Themes::ROSEPINE_DARK;
        SetTheme(m_imguiSettings.theme);
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}