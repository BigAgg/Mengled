#include "editor.h"
#include "engine/components.h"
#include "engine/engine.h"
#include "engine/gui_utilities.h"
#include "engine/resourcemanager.h"
#include "filedialog.h"
#include "themes.h"
#include "utils/logging.h"
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <rlImGui.h>

#define EDITOR_WORKING_DIR "editor/"
#define EDITOR_SETTINGS_FILE EDITOR_WORKING_DIR "settings.bin"

#define FILE_PROJECT_FILTER "Project file:mgd"
#define FILE_PROJECT_FILTER_BIN "Project Binary:bin"
#define FILE_PICTURE_FILTER "Picture:png,jpg,jpeg"
#define FILE_SOUND_FILTER "Sound file:mp3"
#define FILE_SCENE_FILTER "Scene:json,scn"

namespace fs = std::filesystem;

// Helper functions for drawing
void DrawComponentsNode(Scene* scene, Entity entity) {}

void DrawNewEntityCreation(Scene* scene, Entity entity = Entity()) {
  static std::string name;
  ImGui::InputText("Name", &name);
  if (ImGui::Button("Cancel")) {
    name.clear();
    ImGui::CloseCurrentPopup();
  }
  if (name.empty())
    ImGui::BeginDisabled();
  ImGui::SameLine();
  if (!ImGui::Button("Create")) {
    if (name.empty())
      ImGui::EndDisabled();
    return;
  }
  ImGui::CloseCurrentPopup();
  auto c = scene->CreateEntity(name);
  name.clear();
  if (entity) {
    scene->SetParent(c, entity);
  }
}

void DrawEntityNode(Scene* scene, Entity entity) {
  auto& name = entity.GetComponent<NameComponent>();
  ImGui::PushID(entity);
  if (ImGui::TreeNode((void*)(uint64_t)entity.GetComponent<IDComponent>().ID, "%s",
                      name.name.c_str())) {
    auto& rel = entity.GetComponent<RelationshipComponent>();
    if (ImGui::Button("New Child")) {
      ImGui::OpenPopup("New Child");
    }
    if (ImGui::BeginPopupModal("New Child", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      DrawNewEntityCreation(scene, entity);
      ImGui::EndPopup();
    }
    if (ImGui::SameLine(), ImGui::Button("Delete")) {
      scene->DestroyEntity(entity);
    }
    DrawComponentsNode(scene, entity);
    for (auto childID : rel.children) {
      Entity child = scene->GetEntityByUUID(childID);
      if (child)
        DrawEntityNode(scene, child);
    }
    ImGui::TreePop();
  }
  ImGui::PopID();
}

Editor::~Editor() {
  m_wm.clear();
  m_resManager.UnloadAll();
}

void Editor::run() {
  if (!init())
    throw std::runtime_error("[Editor::run] Unable to initialize editor!");
  if (!initImgui())
    throw std::runtime_error("[Editor::run] Unable to initialize ImGui!");
  logging::loginfo("[Editor::run] Mengled Editor %s initialized!", MENGLED_EDITOR_VERSION);
  loop();
  cleanup();
}

void Editor::loop() {
  while (!WindowShouldClose() && !m_close) {
    m_wm.Update();
    BeginDrawing();
    ClearBackground(BLACK);
    rlImGuiBegin();
    taskbar();
    docspacehost();
    objectSelector();
    objectEditor();
    resourceEditor();
    animationEditor();
    sceneEditor();
    scenePreview();
    rlImGuiEnd();
    m_wm.Draw();
    EndDrawing();
  }
}

bool Editor::init() {
  if (!initResourcemanager())
    return false;
  loadSettings();
  std::string windowtitle = "Mengled Editor ";
  windowtitle += MENGLED_EDITOR_VERSION;
  SetWindowTitle(windowtitle.c_str());
  return true;
}

bool Editor::initResourcemanager() {
  m_engine.init();
  if (!fs::exists(EDITOR_WORKING_DIR))
    fs::create_directories(EDITOR_WORKING_DIR);
  return true;
}

bool Editor::initImgui() {
  rlImGuiSetup(false);
  SetTheme(m_imguiSettings.theme);
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  // io.IniFilename = nullptr;  // Disable ini file
  if (fs::exists("fonts/JetBrainsMonoNerdFont-Bold.ttf")) {
    ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/JetBrainsMonoNerdFont-Bold.ttf", 18.0f,
                                                nullptr, io.Fonts->GetGlyphRangesDefault());
    if (font)
      io.FontDefault = font;
    else
      logging::logwarning("[Editor::initImgui] Font could not be loaded: %s",
                          "fonts/JetBrainsMonoNerdFont-Bold.ttf");
  } else {
    logging::logwarning("[Editor::initImgui] Font file does not exist: %s",
                        "fonts/JetBrainsMonoNerdFont-Bold.ttf");
  }
  return true;
}

bool Editor::saveSettings() {
  std::ofstream file;
  file.open(EDITOR_SETTINGS_FILE, std::ios::binary);
  if (!file) {
    logging::logwarning("[Editor::saveSettings] Unable to save editor settings: %s",
                        EDITOR_SETTINGS_FILE);
    return false;
  }
  file.write((char*)&m_imguiSettings, sizeof(m_imguiSettings));
  file.close();
  return true;
}

bool Editor::loadSettings() {
  std::ifstream file;
  file.open(EDITOR_SETTINGS_FILE, std::ios::binary);
  if (!file) {
    logging::logwarning("[Editor::loadSettings] Unable to load editor settings: %s",
                        EDITOR_SETTINGS_FILE);
    return false;
  }
  file.read((char*)&m_imguiSettings, sizeof(m_imguiSettings));
  file.close();
  return true;
}

void Editor::cleanup() {
  saveSettings();
  m_resManager.UnloadAll();
}

void Editor::taskbar() {
  if (ImGui::BeginMainMenuBar()) {
    // File handling
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Project")) {
        logging::loginfo("Selected folder: %s", OpenDirectoryDialog().c_str());
      }
      if (ImGui::MenuItem("Load Project")) {
        auto ss = SceneSerializer(m_engine.sceneManager.GetCurrentScene());
        const std::string& path = OpenFileDialog({FILE_SCENE_FILTER});
        if (!path.empty())
          ss.Deserialize(path);
      }
      if (ImGui::MenuItem("Save as")) {
        auto ss = SceneSerializer(m_engine.sceneManager.GetCurrentScene());
        const std::string& path = SaveFileDialog({FILE_SCENE_FILTER});
        if (!path.empty())
          ss.Serialize(path);
      }
      if (ImGui::MenuItem("Exit"))
        m_close = true;
      ImGui::EndMenu();
    }

    // Windows
    if (ImGui::BeginMenu("Windows")) {
      for (auto& wininfo : m_windows) {
        ImGui::Checkbox(wininfo.first.c_str(), &wininfo.second);
      }
      ImGui::EndMenu();
    }

    // Preferences
    if (ImGui::BeginMenu("Preferences")) {
      // Theme selection
      if (ImGui::BeginMenu("Theme")) {
        if (ImGui::MenuItem("Default", nullptr, m_imguiSettings.theme == 0)) {
          m_imguiSettings.theme = Themes::DEFAULT;
          SetTheme(m_imguiSettings.theme);
        }
        ImGui::SeparatorText("Light themes");
        if (ImGui::MenuItem("Gold Light", nullptr, m_imguiSettings.theme == Themes::GOLD_LIGHT)) {
          m_imguiSettings.theme = Themes::GOLD_LIGHT;
          SetTheme(m_imguiSettings.theme);
        }
        if (ImGui::MenuItem("Purple Light", nullptr,
                            m_imguiSettings.theme == Themes::PURPLE_LIGHT)) {
          m_imguiSettings.theme = Themes::PURPLE_LIGHT;
          SetTheme(m_imguiSettings.theme);
        }
        if (ImGui::MenuItem("Girly Pink", nullptr, m_imguiSettings.theme == Themes::GIRLY_PINK)) {
          m_imguiSettings.theme = Themes::GIRLY_PINK;
          SetTheme(m_imguiSettings.theme);
        }
        if (ImGui::MenuItem("Noctua Light", nullptr,
                            m_imguiSettings.theme == Themes::NOCTUA_LIGHT)) {
          m_imguiSettings.theme = Themes::NOCTUA_LIGHT;
          SetTheme(m_imguiSettings.theme);
        }
        if (ImGui::MenuItem("RosePine Light", nullptr,
                            m_imguiSettings.theme == Themes::ROSEPINE_LIGHT)) {
          m_imguiSettings.theme = Themes::ROSEPINE_LIGHT;
          SetTheme(m_imguiSettings.theme);
        }
        ImGui::SeparatorText("Dark themes");
        if (ImGui::MenuItem("Gold Dark", nullptr, m_imguiSettings.theme == Themes::GOLD_DARK)) {
          m_imguiSettings.theme = Themes::GOLD_DARK;
          SetTheme(m_imguiSettings.theme);
        }
        if (ImGui::MenuItem("Purple Dark", nullptr, m_imguiSettings.theme == Themes::PURPLE_DARK)) {
          m_imguiSettings.theme = Themes::PURPLE_DARK;
          SetTheme(m_imguiSettings.theme);
        }
        if (ImGui::MenuItem("Noctua Dark", nullptr, m_imguiSettings.theme == Themes::NOCTUA_DARK)) {
          m_imguiSettings.theme = Themes::NOCTUA_DARK;
          SetTheme(m_imguiSettings.theme);
        }
        if (ImGui::MenuItem("RosePine Dark", nullptr,
                            m_imguiSettings.theme == Themes::ROSEPINE_DARK)) {
          m_imguiSettings.theme = Themes::ROSEPINE_DARK;
          SetTheme(m_imguiSettings.theme);
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

void Editor::docspacehost() {
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                  ImGuiWindowFlags_NoNavFocus;

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  ImGui::Begin("DockSpaceHost", nullptr, window_flags);
  ImGui::PopStyleVar(2);

  // Important: creates the docking node
  ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0, 0));

  ImGui::End();
}

void Editor::objectSelector() {
  if (!m_windows["Object Selector"])
    return;
  if (ImGui::Begin("Object Selector", &m_windows["Object Selector"])) {
    auto scene = m_engine.sceneManager.GetCurrentScene();
    if (ImGui::Button("New Entity")) {
      ImGui::OpenPopup("New Entity");
    }
    if (ImGui::BeginPopupModal("New Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
      DrawNewEntityCreation(scene);
      ImGui::EndPopup();
    }
    for (auto root : scene->GetRootEntities())
      DrawEntityNode(scene, root);
  }
  ImGui::End();
}

void Editor::objectEditor() {
  if (!m_windows["Object Editor"])
    return;
  if (ImGui::Begin("Object Editor", &m_windows["Object Editor"])) {
  }
  ImGui::End();
}

void Editor::resourceEditor() {
  if (!m_windows["Resource Editor"])
    return;
  if (ImGui::Begin("Resource Editor", &m_windows["Resource Editor"])) {
  }
  ImGui::End();
}

void Editor::animationEditor() {
  if (!m_windows["Animation Editor"])
    return;
  if (ImGui::Begin("Animation Editor", &m_windows["Animation Editor"])) {
  }
  ImGui::End();
}

void Editor::sceneEditor() {
  if (!m_windows["Scene Editor"])
    return;
  if (ImGui::Begin("Scene Editor", &m_windows["Scene Editor"])) {
  }
  ImGui::End();
}

void Editor::scenePreview() {
  if (!m_windows["Scene Preview"])
    return;
  if (ImGui::Begin("Scene Preview", &m_windows["Scene Preview"])) {
  }
  ImGui::End();
}
