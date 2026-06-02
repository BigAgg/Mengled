#include "m_main.h"
#include "utils/logging.h"
#include "utils/timer.h"
#include "engine/gui_utilities.h"
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <raylib.h>
#include <rlImGui.h>

#ifdef MENGLED_DEV
#include "editor/editor.h"
#endif

namespace fs = std::filesystem;

int m_main(int argc, char* argv[]) {
	std::string args;
	for (int i = 0; i < argc; i++) {
		args += "\t\t";
		if(i == 0)
			args += "(executable path): ";
		args += argv[i];
		if(i != argc - 1)
			args += "\n";
	}
  #ifndef NDEBUG
  if (!fs::exists("logs/"))
    fs::create_directories("logs/");
  logging::startlogging("logs/", "log_" + strings::GetTimestamp() + ".txt");
  #endif
	logging::loginfo("Starting Mengled with arguments:\n%s", args.c_str());

	try {
		Mengled m;
		m.run();
	}
	catch (const std::exception& e) {
		logging::logerror("[m_main] %s", e.what());
    logging::backuplog("logs/", true);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

#define SETTINGS_BIN "settings.bin"

void Mengled::run() {
	loadSettings();
	if (!initRaylib())
		throw std::runtime_error("[Mengled::run] Unable to initialize raylib!");
	if (!initImGui())
		throw std::runtime_error("[Mengled::run] Unable to initialize ImGui!");
	if (!initEngine())
		throw std::runtime_error("[Mengled::run] Unable to initialize engine!");
	if (!initGame())
		throw std::runtime_error("[Mengled::run] Unable to initialize game!");
#ifdef MENGLED_DEV
	if (!initEditor())
		throw std::runtime_error("[Mengled::run] Unable to initialize editor!");
#endif
	if (!initResourcemanager())
		throw std::runtime_error("[Mengled::run] Unable to initialize resourcemanager!");
#ifndef MENGLED_DEV
	startupWindow();
#endif
  setupMenus();
	loop();
	saveSettings();
}

bool Mengled::initRaylib(){
	InitWindow(m_settings.w, m_settings.h, "Mengled");
	SetTargetFPS(m_settings.fps);
#ifndef MENGLED_DEV
	SetExitKey(KEY_NULL);
#endif
	if (m_settings.vsync)
		SetWindowState(FLAG_VSYNC_HINT);
	if (m_settings.device != -1)
		SetWindowMonitor(m_settings.device);
	if (m_settings.posx != -1 && m_settings.posy != -1)
		SetWindowPosition(m_settings.posx, m_settings.posy);
	if (m_settings.borderless)
		ToggleBorderlessWindowed();
	return IsWindowReady();
}

bool Mengled::initImGui(){
	return true;
}

bool Mengled::initEngine(){
	return true;
}

bool Mengled::initGame(){
	return true;
}

bool Mengled::initResourcemanager(){
	if (!m_resManager.LoadTexture("main_background", "textures/main_background.png"))
		return false;
	if (!m_resManager.LoadTexture("button", "textures/button.png"))
		return false;
	return true;
}

void Mengled::startupWindow(){
	Timer t;
	t.Start();
	bool stop = false;
	Color c = WHITE;
	c.a = 0;
	Texture bkg = m_resManager.GetTexture("main_background");
	while (!WindowShouldClose() && !stop) {
		if (t.GetElapsedSeconds() > 7.5f)
			stop = true;
		if (c.a < 255)
			c.a++;
		BeginDrawing();
		ClearBackground(BLACK);
		DrawFPS(10, 10);
		DrawTexturePro(bkg,
			{ 0, 0, static_cast<float>(bkg.width), static_cast<float>(bkg.height) },
			{ 0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) },
			{ 0, 0 }, 0.0f, c);
		EndDrawing();
	}
}

void Mengled::loop(){
	Texture bkg = m_resManager.GetTexture("main_background");
  
	while (!WindowShouldClose() && !m_close) {
    if(IsWindowResized()){
      auto ptr = m_wm.GetWindow("Main Menu");
      const Rectangle MENU_BOUNDS = ptr->GetWindowRect();
      ptr->SetPosition(GetScreenWidth() / 2 - MENU_BOUNDS.width / 2, GetScreenHeight() / 2 - MENU_BOUNDS.height / 2);
    }
    m_wm.Update();
		BeginDrawing();
		ClearBackground(GRAY);
		DrawTexturePro(bkg,
			{ 0, 0, static_cast<float>(bkg.width), static_cast<float>(bkg.height) },
			{ 0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) },
			{ 0, 0 }, 0.0f, WHITE);
		m_wm.Draw();
		DrawFPS(10, 10);
		EndDrawing();
	}
}

void Mengled::setupMenus(){
  setupMainMenu();
  setupSettingsMenu();
  m_wm.EnableSingleWindow("Main Menu");
}

void Mengled::setupMainMenu(){
	Texture t = m_resManager.GetTexture("button");
  float buttonypos = 20;
  auto ptr = m_wm.CreateWindow();
  ptr->SetName("Main Menu");
  ptr->SetFlags(
    UiWindowFlags_NoMove |
    UiWindowFlags_NoResize |
    UiWindowFlags_NoTitleBar |
    UiWindowFlags_NoBackground);
  ptr->CreateWidget<Button>(t, Rectangle{ 20, buttonypos, 150, 50 }, "New", []() {
    logging::loginfo("New Game button clicked!");
  });
  buttonypos += 70;
  ptr->CreateWidget<Button>(t, Rectangle{ 20, buttonypos, 150, 50 }, "Load", []() {
    logging::loginfo("Load Game button clicked!");
  });
  buttonypos += 70;
#ifdef MENGLED_DEV
  ptr->CreateWidget<Button>(t, Rectangle{ 20, buttonypos, 150, 50 }, "Editor", []() {
    logging::loginfo("Start Editor button clicked!");
    Editor e;
    e.run();
  });
  buttonypos += 70;
#endif
  ptr->CreateWidget<Button>(t, Rectangle{ 20, buttonypos, 150, 50 }, "Settings",
    [&]() {
      m_wm.EnableSingleWindow("Settings Menu");
  });
  buttonypos += 70;
  ptr->CreateWidget<Button>(t, Rectangle{ 20, buttonypos, 150, 50 }, "Exit", [&]() {
    m_close = true;
  });
  buttonypos += 70;
  // test dropdown
  ptr->CreateWidget<ComboBox>("Resolution", t, Rectangle{ 20, buttonypos, 150, 50 }, std::vector<std::string>{"1920x1080", "1280x720", "800x600"}, 0, [](const std::string& option) {
    logging::loginfo("Selected resolution: %s", option.c_str());
  });

  const Rectangle MENU_BOUNDS = ptr->GetWindowRect();
  ptr->SetPosition(GetScreenWidth() / 2 - MENU_BOUNDS.width / 2, GetScreenHeight() / 2 - MENU_BOUNDS.height / 2);
}

void Mengled::setupSettingsMenu(){
  Texture t = m_resManager.GetTexture("button");
  float buttonypos = 20;
  auto ptr = m_wm.CreateWindow();
  ptr->SetName("Settings Menu");
  ptr->SetFlags(
    UiWindowFlags_NoMove |
    UiWindowFlags_NoResize |
    UiWindowFlags_NoTitleBar |
    UiWindowFlags_NoBackground);
  ptr->CreateWidget<Button>(t, Rectangle{ 20, buttonypos, static_cast<float>(MeasureText("Back to Main Menu", 20) + 20*3), 50 }, "Back to Main Menu", [&]() {
    m_wm.EnableSingleWindow("Main Menu");
  });
  ptr->SetPosition(20, 20);
}

void Mengled::loadSettings(){
	// loading defaults
	m_settings = { 1920, 1080, 60, -1, -1, -1, false, true };
	// Opening file
	std::ifstream file(SETTINGS_BIN, std::ios::binary);
	if (!file)
		return;
	// Reading file in binary
	file.read((char*)&m_settings, sizeof(m_settings));
}

void Mengled::saveSettings(){
	// Opening file
	std::ofstream file(SETTINGS_BIN, std::ios::binary);
	if (!file)
		return;
	// Saving to file in binary
	file.write((char*)&m_settings, sizeof(m_settings));
}

bool Mengled::initEditor(){
	return true;
}

void Mengled::cleanup() {
  m_wm.clear();
}
