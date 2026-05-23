#pragma once

#include <raylib.h>
#include <functional>
#include <string>
#include <vector>
#include <memory>

typedef int UiWindowFlags;      // -> enum UiWindowFlags_		-> Used for UIManager.flags
enum UiWindowFlags_ {
	UiWindowFlags_None									= 0,
  UiWindowFlags_NoTitleBar						= 1 << 0,		// Disable title-bar
	UiWindowFlags_NoResize							= 1 << 1,		// Disable user resizing
	UiWindowFlags_NoMove								= 1 << 2,		// Disable moving window
	UiWindowFlags_NoScrollbar						= 1 << 3,		// Disable Scrollbars (Can still scroll just no draw)
	UiWindowFlags_NoScrollWithMouse			= 1 << 4,		// Disable scrolling with mouse
	UiWindowFlags_NoCollapse						= 1 << 5,		// Disable collapsing window by double clicking
	UiWindowFlags_AlwaysAutoResize			= 1 << 6,		// Resize every window to its content every frame
	UiWindowFlags_NoBackground					= 1 << 7,		// Disable drawing background colow
	UiWindowFlags_NoSavedSettings				= 1 << 8,		// Don't load/save settings in .ini file
	UiWindowFlags_NoMouseInputs					= 1 << 9,		// Disable mouse inputs and hovering
	UiWindowFlags_MenuBar								= 1 << 10,	// Has a menu-bar
	UiWindowFlags_HorizontalScrollbar		= 1 << 11,	// Allow horizontal scrollbar to appear (off by default)
	UiWindowFlags_NoBringToFrontOnFocus	= 1 << 12,	// Does not bring window in front on focus
	UiWindowFlags_NoDecoration					= 1 << 13,	// Disable background, header, outlines
	UiWindowFlags_NoInputs							= 1 << 14,	// Disable any user input
};



class Widget {
public:
	virtual ~Widget() = default;

	virtual void Update() = 0;
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

	void Update();
	void Draw() const;
	Rectangle GetBounds() const;

	void SetPosition(float x, float y);
	void SetSize(float width, float height);

	void SetText(const std::string& text);

private:
	Rectangle bounds;
	Rectangle trect;
	Texture texture;
	std::string label;
	Callback callback;
	bool hovered = false;
	bool pressed = false;
};

class Checkbox : public Widget {
public:
	using Callback = std::function<void()>;
	Checkbox(Texture& t, Vector2 position, float size, std::string text, bool* val, Callback onClick = nullptr);

	void Update();
	void Draw() const;
	Rectangle GetBounds() const;

	void SetPosition(float x, float y);
	void SetSize(float size);

	void SetText(const std::string& text);

private:
	Rectangle bounds;
	Rectangle trect;
	Texture texture;
	std::string label;
	Callback callback;
	bool hovered = false;
	bool pressed = false;
	bool *val = nullptr;
};

class InputInt : public Widget {
public:
	using Callback = std::function<void()>;
	InputInt(Texture& t, Rectangle bounds, std::string text, int* val, int step = 1, int stepshift = 10, Callback onClick = nullptr);

	void Update();
	void Draw() const;
	Rectangle GetBounds() const;

	void SetPosition(float x, float y);
	void SetSize(float width, float height);

	void SetText(const std::string& text);
	
private:
	Rectangle bounds;
	Rectangle trect;
	Texture texture;
	std::string label;
	Callback callback;
	bool hovered = false;
	bool pressed = false;
	int* val = nullptr;
	int step;
	int stepshift;
	Button stepPlus;
	Button stepMinus;
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
		return ptr;
	}

	void Update();
	void Draw();

	void clear();

	void SetFlags(UiWindowFlags flags);
	void SetFlag(UiWindowFlags flag);
	void ClearFlag(UiWindowFlags flag);
	bool IsFlagActive(UiWindowFlags flag);

private:
	std::vector<std::unique_ptr<Widget>> widgets;
	Rectangle window;
	Rectangle logicalWindow;	// logical size (max width but not rendered)
	bool active;
	UiWindowFlags flags;
};
