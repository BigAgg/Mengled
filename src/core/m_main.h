#pragma once

#define MENGLED_DEV

#include "engine/gui_utilities.h"
#include "engine/resourcemanager.h"

int m_main(int argc, char *argv[]);

class Mengled {
public:
  void run();

private:
  // initializing process
  bool initRaylib();
  bool initResourcemanager();
  void startupWindow();

  // Readjusting functions
  void handleNewWindowsettings();

  // Main loop
  void loop();

  // Menu setups
  void setupMenus();
  void setupMainMenu();
  void setupSettingsMenu();

  // Loading / Saving core ressources
  void loadSettings();
  void saveSettings();

  void cleanup();

  // Run tests
  void tests();

  struct {
    int w, h, fps, device, posx, posy;
    bool borderless, vsync, maximized;
  } m_settings;

  bool m_close = false;
  ResourceManager m_resManager;
  WindowManager m_wm;
};