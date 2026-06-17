#pragma once

#include "engine/engine.h"
#include "engine/gui_utilities.h"
#include "engine/resourcemanager.h"
#include <map>
#include <string>

#define MENGLED_EDITOR_VERSION "v0.1.0"

class Editor {
public:
  ~Editor();

  void run();

private:
  void loop();

  bool init();
  bool initResourcemanager();
  bool initImgui();

  bool loadSettings();
  bool saveSettings();

  void cleanup();

  // Imgui windows
  void taskbar();
  void docspacehost();
  void objectSelector();
  void objectEditor();
  void resourceEditor();
  void animationEditor();
  void sceneEditor();
  void scenePreview();

  bool m_close = false;
  ResourceManager m_resManager;
  WindowManager m_wm;
  std::map<std::string, bool> m_windows = {{"Object Selector", true}, {"Object Editor", true},
                                           {"Resource Editor", true}, {"Animation Editor", false},
                                           {"Scene Editor", true},    {"Scene Preview", true}};
  struct {
    unsigned int theme = 0;
  } m_imguiSettings;

  // Engine handling
  Engine m_engine;
};