#pragma once

#include "engine/gui_utilities.h"
#include "engine/resourcemanager.h"

class Editor {
public:
  ~Editor();

  void run();
private:
  void loop();

  bool init();
  bool initResourcemanager();
  bool initImgui();

  // Imgui windows
  void taskbar();

  bool m_close = false;
  ResourceManager m_resManager;
  WindowManager m_wm;
  struct {
    unsigned int theme = 0;
  } m_imguiSettings;
};