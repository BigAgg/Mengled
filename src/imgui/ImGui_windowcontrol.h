#pragma once
#include <functional>
#include <map>
#include <string>
#include <unordered_map>

struct WindowInformation {
  bool open = false;
  int flags;
  std::function<void()> function;

  explicit operator bool() { return open; }
};

class WindowControl {
public:
  static WindowControl& Get() {
    static WindowControl instance;
    return instance;
  }

  void RegisterWindow(const std::string& name, bool open,
                      std::function<void()> function, int flags = 0);
  void RegisterMenu(const std::string& name,
                    std::function<void()> function);
  void ToggleWindow(const std::string& name);
  void SetWindowState(const std::string& name, bool open);
  WindowInformation& GetWindowInfo(const std::string& name);
  void DrawWindows();
  void DrawMainMenu();
  void ToggleWindow();
  void ThemeSelector();
  void EditWindowFlags();
  void SaveSettings();
  void LoadSettings();
  void ToggleThemeSelect();
  void ToggleEditFlags();
  void ToggleWindowToggle();

  const size_t size() const { return mRegistry.size(); }

private:
  std::unordered_map<std::string, WindowInformation> mRegistry;
  std::map<std::string, std::function<void()>> mMenuBarRegistry;
  std::string mSelectedWindow;
  unsigned char mTheme = 0;
  bool mEditFlagsOpen = false;
  bool mThemeSelectOpen = false;
  bool mWindowToggleOpen = false;
  void CheckboxFlags(const char* label, int* flags, int flag);

public:
  WindowControl(const WindowControl&) = delete;
  WindowControl& operator=(const WindowControl&) = delete;
  WindowControl(WindowControl&&) = delete;
  WindowControl& operator=(WindowControl&&) = delete;

private:
  WindowControl() = default;
  ~WindowControl() = default;
};
