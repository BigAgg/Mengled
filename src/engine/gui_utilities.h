#pragma once

#include <algorithm>
#include <raylib.h>
#include <functional>
#include <string>
#include <vector>
#include <memory>

typedef int UiWindowFlags;      // -> enum UiWindowFlags_		-> Used for UIManager.flags
enum UiWindowFlags_ {
	UiWindowFlags_None					= 0,
  UiWindowFlags_NoTitleBar			= 1 << 0,		// Disable title-bar
	UiWindowFlags_NoResize				= 1 << 1,		// Disable user resizing
	UiWindowFlags_NoMove				= 1 << 2,		// Disable moving window
	UiWindowFlags_NoScrollbar			= 1 << 3,		// Disable Scrollbars (Can still scroll just no draw)
	UiWindowFlags_NoScrollWithMouse		= 1 << 4,		// Disable scrolling with mouse
	UiWindowFlags_NoCollapse			= 1 << 5,		// Disable collapsing window by double clicking
	UiWindowFlags_AlwaysAutoResize		= 1 << 6,		// Resize every window to its content every frame
	UiWindowFlags_NoBackground			= 1 << 7,		// Disable drawing background colow
	UiWindowFlags_NoSavedSettings		= 1 << 8,		// Don't load/save settings in .ini file
	UiWindowFlags_NoMouseInputs			= 1 << 9,		// Disable mouse inputs and hovering
	UiWindowFlags_MenuBar				= 1 << 10,	// Has a menu-bar
	UiWindowFlags_HorizontalScrollbar	= 1 << 11,	// Allow horizontal scrollbar to appear (off by default)
	UiWindowFlags_NoBringToFrontOnFocus	= 1 << 12,	// Does not bring window in front on focus
	UiWindowFlags_NoDecoration			= 1 << 13,	// Disable background, header, outlines
	UiWindowFlags_NoInputs				= 1 << 14,	// Disable any user input
};

struct Style{
	Color background = DARKGRAY;
	Color foreground = GRAY;
	Color accent = ORANGE;
	float borderSize = 2.0f;
  float borderRounding = 0.1f;
	Color borderColor = BLACK;
	int fontSize = 15;
	Color text = WHITE;
	Color highlighted = LIGHTGRAY;
	Color inactive = {130, 130, 130, 200};
  float spacing = 10.0f;
  float titlebarHeight = 20.0f;
};

class Widget {
public:
	virtual ~Widget() = default;

	virtual void Update(const Vector2 mousePos) = 0;
	virtual void Draw() const = 0;
	virtual Rectangle GetBounds() const = 0;

	void SetEnabled(bool value);

	bool visible = true;
	bool enabled = true;
};

class Button : public Widget {
public:
	using Callback = std::function<void()>;
	Button(Texture &t, Rectangle rect, std::string text, Callback onClick);

	void Update(const Vector2 mousePos);
	void Draw() const;
	Rectangle GetBounds() const;

	void SetPosition(float x, float y);
	void SetSize(float width, float height);

	void SetText(const std::string& text);

private:
	Texture texture = {};
	Rectangle bounds = {0, 0,0 ,0};
	std::string label = "";
	Rectangle trect = {0, 0, 0, 0};
	Callback callback = {};
	bool hovered = false;
	bool pressed = false;
};

class ComboBox : public Widget {
  public:
    using Callback = std::function<void(const std::string& option)>;
    ComboBox(const std::string& label, Texture& t, Rectangle rect, std::vector<std::string> options, int selectedIndex, Callback onChange);

    void Update(const Vector2 mousePos);
    void Draw() const;
    Rectangle GetBounds() const;

    void SetPosition(float x, float y);
    void SetSize(float width, float height);

    void SetOptions(const std::vector<std::string>& options);
    std::vector<std::string> GetOptions();
    void SetSelectedIndex(int index);
    int GetSelectedIndex() const;
    void SetText(const std::string& text);
    void SetLabel(const std::string& text);
    bool Selecting() const;

  private:
    std::string label;
    Texture texture;
    Rectangle bounds;
    std::vector<std::string> options;
    int selectedIndex = -1;
    Callback callback;
    Rectangle trect;
    bool hovered = false;
    bool pressed = false;
    bool selecting = false;
};

class Checkbox : public Widget {
public:
	using Callback = std::function<void()>;
	Checkbox(Texture& t, Vector2 position, float size, std::string text, bool* val, Callback onClick = nullptr);

	void Update(const Vector2 mousePos);
	void Draw() const;
	Rectangle GetBounds() const;

	void SetPosition(float x, float y);
	void SetSize(float size);

	void SetText(const std::string& text);

private:
	Texture texture;
	std::string label;
	bool *val = nullptr;
	Callback callback;
	Rectangle bounds;
	Rectangle trect;
	bool hovered = false;
	bool pressed = false;
};

class InputInt : public Widget {
public:
	using Callback = std::function<void()>;
	InputInt(Texture& t, Rectangle bounds, std::string text, int* val, int step = 1, int stepshift = 10, Callback onClick = nullptr);

	void Update(const Vector2 mousePos);
	void Draw() const;
	Rectangle GetBounds() const;

	void SetPosition(float x, float y);
	void SetSize(float width, float height);

	void SetText(const std::string& text);
	
private:
	Texture texture;
	Rectangle bounds;
	std::string label;
	int* val = nullptr;
	int step;
	int stepshift;
	Callback callback;
	Rectangle trect;
	Button stepPlus;
	Button stepMinus;
	bool hovered = false;
	bool pressed = false;
	std::string valStr;
};

class InputDouble : public Widget {
public:
	using Callback = std::function<void()>;
	InputDouble(Texture& t, Rectangle bounds, std::string text, double* val, double step = 0.1, double stepshift = 1.0, Callback onClick = nullptr);

	void Update(const Vector2 mousePos);
	void Draw() const;
	Rectangle GetBounds() const;

	void SetPosition(float x, float y);
	void SetSize(float width, float height);

	void SetText(const std::string& text);
	void SetFormat(const std::string& format) {
		formatStr = format;
	}

	std::string FormatValue() const;

private:
	Texture texture;
	Rectangle bounds;
	std::string label;
	double* val = nullptr;
	double step;
	double stepshift;
	Callback callback;
	Rectangle trect;
	Button stepPlus;
	Button stepMinus;
	bool hovered = false;
	bool pressed = false;
	std::string valStr;
	std::string formatStr = "%.2f";
};

class InputFloat : public InputDouble {
public:
	InputFloat(Texture& t, Rectangle bounds, std::string text, float* val, float step = 0.1f, float stepshift = 1.0f, Callback onClick = nullptr)
		: InputDouble(t, bounds, text, reinterpret_cast<double*>(val), step, stepshift, onClick) {}
	
};

class InputString : public Widget {
public:
	using Callback = std::function<void()>;
	InputString(Texture& t, Rectangle bounds, std::string text, std::string* val, size_t limit=256, Callback onClick = nullptr);

	void Update(const Vector2 mousePos);
	void Draw() const;
	Rectangle GetBounds() const;

	void SetPosition(float x, float y);
	void SetSize(float width, float height);

	void SetText(const std::string& text);
private:
	Texture texture;
	Rectangle bounds;
	std::string label;
	std::string* val = nullptr;
	size_t limit;
	Callback callback;
	Rectangle trect;
	bool hovered = false;
	bool pressed = false;
	std::string inputStr;
};

class UIManager {
public:
	UIManager() = default;
	~UIManager() = default;

	UIManager(const UIManager&) = delete;
	UIManager& operator=(const UIManager&) = delete;

	template<typename T, typename... Args>
	T* CreateWidget(Args&&... args) {
		static_assert(std::is_base_of_v<Widget, T>,
			"T must derive from Widget");

		auto widget = std::make_unique<T>(
			std::forward<Args>(args)...);

		T* ptr = widget.get();
		widgets.push_back(std::move(widget));
    Vector2 tl = {ptr->GetBounds().x, ptr->GetBounds().y};
    Vector2 br = {ptr->GetBounds().x + ptr->GetBounds().width, ptr->GetBounds().y + ptr->GetBounds().height};
    for(auto& w : widgets){
      const auto& rect = w->GetBounds();
      if(rect.x + rect.width > br.x)
        br.x = rect.x + rect.width;
      if(rect.y + rect.height > br.y)
        br.y = rect.y + rect.height; 
      if(rect.x < tl.x)
        tl.x = rect.x - style.spacing;
      if(rect.y < tl.y)
        tl.y = rect.y - style.spacing;
    }
    logicalWindow.width = br.x + style.spacing;
    logicalWindow.height = br.y + style.spacing;
    logicalWindow.x = 0;
    logicalWindow.y = 0;
    window.width = logicalWindow.width;
    window.height = logicalWindow.height;
    UnloadRenderTexture(windowTexture);
    windowTexture = LoadRenderTexture(
      static_cast<int>(window.width),
      static_cast<int>(window.height));
    // sort the comboboxes to be at the end of the vector and order them to have the highest y pos to be at the end
    std::stable_partition(widgets.begin(), widgets.end(), [](const std::unique_ptr<Widget>& widget){
      return dynamic_cast<ComboBox*>(widget.get()) == nullptr;
    });
    for(size_t i = 0; i < widgets.size(); i++){
      if(dynamic_cast<ComboBox*>(widgets[i].get()) != nullptr){
        for(size_t j = i + 1; j < widgets.size(); j++){
          if(dynamic_cast<ComboBox*>(widgets[j].get()) != nullptr){
            if(widgets[i]->GetBounds().y < widgets[j]->GetBounds().y){
              std::swap(widgets[i], widgets[j]);
            }
          }
        }
      }
    }
    return ptr;
	}

	void Update();
	void Draw();

	void clear();
  void SetPosition(float x, float y);
  void SetSize(float width, float height);
  void SetName(const std::string& name);
  std::string GetName() const;

	void SetFlags(UiWindowFlags flags);
	void SetFlag(UiWindowFlags flag);
	void ClearFlag(UiWindowFlags flag);
	bool IsFlagActive(UiWindowFlags flag);

  void SetEnabled(bool value);
  bool GetEnabled() const;
  void SetActive(bool value);
  bool GetActive() const;


  Style* GetStyle() {
    return &style;
  }
  Rectangle GetWindowRect() const {
    return window;
  }

private:
  void m_DrawTitlebar();
	std::vector<std::unique_ptr<Widget>> widgets;
  RenderTexture2D windowTexture;
	Rectangle window = {0.0f, 0.0f, 300.0f, 200.0f};	// actual rendered size
  Rectangle logicalWindow = {0.0f, 0.0f, 300.0f, 200.0f};	// logical size (max width but not rendered)
	bool active = true;
  bool enabled = true;
  bool dragging = false;
	UiWindowFlags flags = UiWindowFlags_None;
  Style style;
  std::string name = "Window";
};

class WindowManager {
public:
  WindowManager() = default;
  ~WindowManager() = default;

  WindowManager(const WindowManager&) = delete;
  WindowManager& operator=(const WindowManager&) = delete;

  template<typename... Args>
  UIManager* CreateWindow(Args&&... args) {
    auto window = std::make_unique<UIManager>(
      std::forward<Args>(args)...);

    UIManager* ptr = window.get();
    windows.push_back(std::move(window));
    return ptr;
  }
  void clear();
  void Update();
  void Draw();

  UIManager* GetActiveWindow(){
    if(windows.empty())
      return nullptr;
    return windows.back().get();
  }
  UIManager* GetWindow(size_t index){
    if(index >= windows.size())
      return nullptr;
    return windows[index].get();
  }
  UIManager* GetWindow(const std::string& name){
    for(auto& window : windows){
      if(window->GetName() == name)
        return window.get();
    }
    return nullptr;
  }
  UIManager* GetWindowIndex(const std::string& name){
    for(size_t i = 0; i < windows.size(); i++){
      if(windows[i]->GetName() == name)
        return windows[i].get();
    }
    return nullptr;
  }

  void EnableSingleWindow(const std::string& name){
    int index = -1;
    for(size_t i = 0; i < windows.size(); i++){
      if(windows[i]->GetName() == name){
        index = i;
      }
      windows[i]->SetEnabled(false);
    }
    if(index != -1){
      windows[index]->SetEnabled(true);
      windows[index]->SetActive(true);
      std::swap(windows[windows.size() - 1], windows[index]);
    }
  }

private:
  std::vector<std::unique_ptr<UIManager>> windows;
};