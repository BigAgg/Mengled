#include "m_main.h"
#include "engine/engine.h"
#include "engine/gui_utilities.h"
#include "utils/logging.h"
#include "utils/timer.h"
#include <filesystem>
#include <fstream>
#include <raylib.h>
#include <rlImGui.h>
#include <stdexcept>

#ifdef MENGLED_DEV
#include "editor/editor.h"
#endif
#include <string>
#include <cstdlib>
#include <exception>
#include <cstdio>
#include <vector>

namespace fs = std::filesystem;

int m_main(int argc, char* argv[]) {
  std::string args;
  for (int i = 0; i < argc; i++) {
    args += "\t\t";
    if (i == 0)
      args += "(executable path): ";
    args += argv[i];
    if (i != argc - 1)
      args += "\n";
  }
#ifdef NDEBUG
  if (!fs::exists("crashes/"))
    fs::create_directories("crashes/");
  if (!fs::exists("logs/"))
    fs::create_directories("logs/");
  const auto& logfiles = fs::directory_iterator("logs/");
  std::vector<std::string> todelete;
  for (const auto& logfile : logfiles) {
    todelete.push_back(logfile.path().string());
  }
  for (size_t i = todelete.size(); i > 4; i--) {
    std::remove(todelete[i - 1].c_str());
  }
  logging::startlogging("logs/",
                        "log_" + strings::GetTimestamp() + ".txt");
#endif
  logging::loginfo("Starting Mengled with arguments:\n%s", args.c_str());

  try {
    Mengled m;
    m.run();
  } catch (const std::exception& e) {
    logging::logerror("[m_main] %s", e.what());
    logging::stoplogging();
    logging::backuplog("crashes/", true);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

constexpr auto SETTINGS_BIN = "settings.bin";

void Mengled::run() {
  tests();
  loadSettings();
  if (!initRaylib())
    throw std::runtime_error(
        "[Mengled::run] Unable to initialize raylib!");
  if (!initResourcemanager())
    throw std::runtime_error(
        "[Mengled::run] Unable to initialize resourcemanager!");
#ifndef MENGLED_DEV
  startupWindow();
#endif
  setupMenus();
  loop();
  saveSettings();
}

bool Mengled::initRaylib() const {
  std::string windowtitle = "Mengled v";
  windowtitle += APP_VERSION;
  InitWindow(m_settings.w, m_settings.h, windowtitle.c_str());
  SetTargetFPS(m_settings.fps);
  InitAudioDevice();
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
  else if (m_settings.maximized) {
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    MaximizeWindow();
    ClearWindowState(FLAG_WINDOW_RESIZABLE);
  }
  if (m_settings.h >= GetMonitorHeight(GetCurrentMonitor()) &&
      !m_settings.maximized && !m_settings.borderless) {
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    MaximizeWindow();
    ClearWindowState(FLAG_WINDOW_RESIZABLE);
  }
  return IsWindowReady();
}

bool Mengled::initResourcemanager() {
  if (!m_resManager.LoadTexture("main_background",
                                "textures/main_background.png"))
    return false;
  if (!m_resManager.LoadTexture("button", "textures/button.png"))
    return false;
  return true;
}

void Mengled::startupWindow() {
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
                   {0, 0, static_cast<float>(bkg.width),
                    static_cast<float>(bkg.height)},
                   {0, 0, static_cast<float>(GetScreenWidth()),
                    static_cast<float>(GetScreenHeight())},
                   {0, 0}, 0.0f, c);
    EndDrawing();
  }
}

void Mengled::handleNewWindowsettings() {}

void Mengled::loop() {
  Texture bkg = m_resManager.GetTexture("main_background");

  while (!WindowShouldClose() && !m_close) {
    if (IsWindowResized()) {
      auto ptr = m_wm.GetWindow("Main Menu");
      const Rectangle MENU_BOUNDS = ptr->GetWindowRect();
      ptr->SetPosition(GetScreenWidth() / static_cast<float>(2) -
                           MENU_BOUNDS.width / 2,
                       GetScreenHeight() / static_cast<float>(2) -
                           MENU_BOUNDS.height / 2);
    }
    m_wm.Update();
    BeginDrawing();
    ClearBackground(GRAY);
    DrawTexturePro(bkg,
                   {0, 0, static_cast<float>(bkg.width),
                    static_cast<float>(bkg.height)},
                   {0, 0, static_cast<float>(GetScreenWidth()),
                    static_cast<float>(GetScreenHeight())},
                   {0, 0}, 0.0f, WHITE);
    m_wm.Draw();
    DrawFPS(10, 10);
    EndDrawing();
  }
}

void Mengled::setupMenus() {
  setupMainMenu();
  setupSettingsMenu();
  m_wm.EnableSingleWindow("Main Menu");
}

void Mengled::setupMainMenu() {
  Texture t = m_resManager.GetTexture("button");
  float buttonypos = 20;
  auto ptr = m_wm.CreateWindow();
  ptr->SetName("Main Menu");
  ptr->SetFlags(UiWindowFlags_NoMove | UiWindowFlags_NoResize |
                UiWindowFlags_NoTitleBar | UiWindowFlags_NoBackground);
  ptr->CreateWidget<Button>(
      t, Rectangle{20, buttonypos, 150, 50}, "New",
      []() { logging::loginfo("New Game button clicked!"); });
  buttonypos += 70;
  ptr->CreateWidget<Button>(
      t, Rectangle{20, buttonypos, 150, 50}, "Load",
      []() { logging::loginfo("Load Game button clicked!"); });
  buttonypos += 70;
#ifdef MENGLED_DEV
  ptr->CreateWidget<Button>(t, Rectangle{20, buttonypos, 150, 50},
                            "Editor", []() {
                              Editor e;
                              SetWindowState(FLAG_WINDOW_RESIZABLE);
                              e.run();
                              ClearWindowState(FLAG_WINDOW_RESIZABLE);
                              std::string windowtitle = "Mengled ";
                              windowtitle += APP_VERSION;
                              SetWindowTitle(windowtitle.c_str());
                            });
  buttonypos += 70;
#endif
  ptr->CreateWidget<Button>(
      t, Rectangle{20, buttonypos, 150, 50}, "Settings",
      [&]() { m_wm.EnableSingleWindow("Settings Menu"); });
  buttonypos += 70;
  ptr->CreateWidget<Button>(t, Rectangle{20, buttonypos, 150, 50}, "Exit",
                            [&]() { m_close = true; });
  buttonypos += 70;

  const Rectangle MENU_BOUNDS = ptr->GetWindowRect();
  ptr->SetPosition(
      GetScreenWidth() / static_cast<float>(2) - MENU_BOUNDS.width / 2,
                   GetScreenHeight() / static_cast<float>(2) -
                       MENU_BOUNDS.height / 2);
}

void Mengled::setupSettingsMenu() {
  Texture t = m_resManager.GetTexture("button");
  float buttonypos = 20;
  auto ptr = m_wm.CreateWindow();
  ptr->SetName("Settings Menu");
  ptr->SetFlags(UiWindowFlags_NoMove | UiWindowFlags_NoResize |
                UiWindowFlags_NoTitleBar);
  auto c = ptr->GetStyle();
  c->background = GRAY;
  c->borderColor = WHITE;
  c->background.a = 200;
  //  test dropdown
  auto cb = ptr->CreateWidget<ComboBox>(
      "Resolution", t, Rectangle{20, buttonypos, 150, 50},
      std::vector<std::string>{"2560x1440", "1920x1080", "1280x720"}, 0,
      [&](const std::string& option) {
        logging::loginfo("Selected resolution: %s", option.c_str());
        const std::string delimiter = "x";
        size_t pos = option.find(delimiter);
        if (pos == std::string::npos)
          return;
        m_settings.w = std::stoi(option.substr(0, pos));
        m_settings.h = std::stoi(option.substr(pos + delimiter.length()));
        if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE))
          return;
        if (IsWindowMaximized()) {
          SetWindowState(FLAG_WINDOW_RESIZABLE);
          RestoreWindow();
          ClearWindowState(FLAG_WINDOW_RESIZABLE);
        }
        SetWindowSize(m_settings.w, m_settings.h);
        if (m_settings.h >= GetMonitorHeight(GetCurrentMonitor())) {
          SetWindowState(FLAG_WINDOW_RESIZABLE);
          MaximizeWindow();
          ClearWindowState(FLAG_WINDOW_RESIZABLE);
        }
        auto ptr = m_wm.GetWindow("Main Menu");
        const Rectangle MENU_BOUNDS = ptr->GetWindowRect();
        ptr->SetPosition(GetScreenWidth() / static_cast<float>(2) -
                             MENU_BOUNDS.width / 2,
                         GetScreenHeight() / static_cast<float>(2) -
                             MENU_BOUNDS.height / 2);
      });
  const std::string res =
      std::to_string(m_settings.w) + "x" + std::to_string(m_settings.h);
  const auto& options = cb->GetOptions();
  for (size_t i = 0; i < options.size(); i++) {
    if (options[i] != res)
      continue;
    cb->SetSelectedIndex(i);
  }
  buttonypos += 70;
  ptr->CreateWidget<Checkbox>(
      t, Vector2(20, buttonypos), 25, "Borderless Windowed",
      &m_settings.borderless, [&]() {
        if (m_settings.borderless !=
            IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE))
          ToggleBorderlessWindowed();
      });
  buttonypos += 70;
  ptr->CreateWidget<Button>(
      t,
      Rectangle{20, buttonypos,
                static_cast<float>(MeasureText("Back", 20) + 20 * 3), 50},
      "Back", [&]() { m_wm.EnableSingleWindow("Main Menu"); });
  ptr->SetPosition(20, 20);
  buttonypos += 70;
}

void Mengled::loadSettings() {
  // loading defaults
  m_settings = {1920, 1080, 60, -1, -1, -1, false, true, true};
  // Opening file
  std::ifstream file(SETTINGS_BIN, std::ios::binary);
  if (!file)
    return;
  // Reading file in binary
  file.read((char*)&m_settings, sizeof(m_settings));
}

void Mengled::saveSettings() {
  // Retrieve current settings
  m_settings.borderless = IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE);
  if (m_settings.borderless)
    ToggleBorderlessWindowed();
  m_settings.maximized = IsWindowMaximized();
  if (m_settings.maximized)
    RestoreWindow();
  m_settings.device = GetCurrentMonitor();
  m_settings.h = GetScreenHeight();
  m_settings.w = GetScreenWidth();
  m_settings.vsync = IsWindowState(FLAG_VSYNC_HINT);
  m_settings.posx = GetWindowPosition().x;
  m_settings.posy = GetWindowPosition().y;
  // Opening file
  std::ofstream file(SETTINGS_BIN, std::ios::binary);
  if (!file)
    return;
  // Saving to file in binary
  file.write((char*)&m_settings, sizeof(m_settings));
}

void Mengled::cleanup() {
  CloseAudioDevice();
  m_wm.clear();
  m_resManager.UnloadAll();
}

void Mengled::tests() { TestEngine(); }
