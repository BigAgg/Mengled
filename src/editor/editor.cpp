#include "editor.h"
#include "engine/gui_utilities.h"
#include "engine/resourcemanager.h"
#include "utils/logging.h"
#include "themes.h"
#include <filesystem>
#include <rlImGui.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

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
    if(ImGui::BeginMenu("File")){
      if(ImGui::MenuItem("Exit"))
        m_close = true;
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}