#pragma once

#include "engine/gui_utilities.h"
#include "engine/resourcemanager.h"

int m_main(int argc, char *argv[]);

class Mengled {
public:
  void run();

private:
  // initializing process
  bool initRaylib() const;
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
    int w = 1080;
    int h = 720;
    int fps = 60;
    int device = 0;
    int posx = 0;
    int posy = 0;
    bool borderless = false;
    bool vsync = true;
    bool maximized = false;
  } m_settings;

  bool m_close = false;
  ResourceManager m_resManager;
  WindowManager m_wm;
};