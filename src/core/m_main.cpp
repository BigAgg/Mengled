#include "m_main.h"
#include "utils/logging.h"
#include "utils/timer.h"
#include "engine/gui_utilities.h"
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <raylib.h>
#include <rlImGui.h>

namespace fs = std::filesystem;

int m_main(int argc, char* argv[]) {
	try {
		Mengled m;
		m.run();
	}
	catch (const std::exception& e) {
		logging::logerror("[m_main] %s", e.what());
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
	Texture t = m_resManager.GetTexture("button");
	Texture bkg = m_resManager.GetTexture("main_background");
	UIManager uim;
	uim.CreateWidget<Button>(t, Rectangle{ 10, 10, 200, 50 }, "Test Button", []() {
		logging::loginfo("Button clicked!");
	});
	while (!WindowShouldClose()) {
		uim.Update();
		BeginDrawing();
		ClearBackground(GRAY);
		DrawTexturePro(bkg,
			{ 0, 0, static_cast<float>(bkg.width), static_cast<float>(bkg.height) },
			{ 0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) },
			{ 0, 0 }, 0.0f, WHITE);
		uim.Draw();
		DrawFPS(10, 10);
		EndDrawing();
	}
	uim.clear();
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
}
