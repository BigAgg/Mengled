#pragma once

#define MENGLED_DEV

#include "engine/resourcemanager.h"
#include "engine/gui_utilities.h"

int m_main(int argc, char* argv[]);

class Mengled {
public:
	void run();
private:
	// initializing process
	bool initRaylib();
	bool initImGui();
	bool initEngine();
	bool initGame();
	bool initResourcemanager();
	void startupWindow();

	// Main loop
	void loop();

  // Menu setups
  void setupMenus();
  void setupMainMenu();
  void setupSettingsMenu();

	// Loading / Saving core ressources
	void loadSettings();
	void saveSettings();
	// Developer stuff
#ifdef MENGLED_DEV
	bool initEditor();
#endif

	void cleanup();

	struct {
		int w, h, fps, device, posx, posy;
		bool borderless, vsync;
	}m_settings;

  bool m_close = false;
	ResourceManager m_resManager;
  WindowManager m_wm;
};