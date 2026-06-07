#include "gui_utilities.h"

#ifndef FONT_SIZE
#define FONT_SIZE 20
#endif

#include "utils/logging.h"
#include <cstdio>
#include <imgui.h>

Button::Button(Texture &t, Rectangle rect, std::string text, Callback onClick)
    : texture(t), bounds(rect), label(std::move(text)),
      callback(std::move(onClick)) {
  trect.x = 0;
  trect.y = 0;
  trect.width = static_cast<float>(t.width);
  trect.height = static_cast<float>(t.height);
}

void Button::Update(const Vector2 mousePos) {
  if (!enabled)
    return;

  hovered = CheckCollisionPointRec(mousePos, bounds);
  if (!hovered)
    return;

  pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  if (pressed && callback) {
    hovered = false;
    callback();
  }
}

void Button::Draw() const {
  Color color = GRAY;

  if (!enabled)
    color = DARKGRAY;
  else if (hovered)
    color = WHITE;

  DrawTexturePro(texture, trect, bounds, {0, 0}, 0.0f, color);

  int textWidth = MeasureText(label.c_str(), FONT_SIZE);

  DrawText(label.c_str(),
           static_cast<int>(bounds.x + (bounds.width - textWidth) / 2),
           static_cast<int>(bounds.y + bounds.height / 2 - FONT_SIZE / 2),
           FONT_SIZE, WHITE);
}

Rectangle Button::GetBounds() const { return bounds; }

void Button::SetPosition(float x, float y) {
  bounds.x = x;
  bounds.y = y;
}

void Button::SetSize(float width, float height) {
  bounds.width = width;
  bounds.height = height;
}

void Button::SetText(const std::string &text) { label = text; }

ComboBox::ComboBox(const std::string &label, Texture &t, Rectangle rect,
                   std::vector<std::string> options, int selectedIndex,
                   Callback onChange)
    : label(label), texture(t), bounds(rect), options(std::move(options)),
      selectedIndex(selectedIndex), callback(onChange) {
  trect.x = 0;
  trect.y = 0;
  trect.width = static_cast<float>(t.width);
  trect.height = static_cast<float>(t.height);
}

void ComboBox::Update(const Vector2 mousePos) {
  if (!enabled)
    return;

  hovered = CheckCollisionPointRec(mousePos, bounds);

  pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  if (selecting) {
    for (size_t i = 0; i < options.size(); i++) {
      Rectangle optionRect = {bounds.x, bounds.y + bounds.height * (i + 1),
                              bounds.width, bounds.height};
      if (CheckCollisionPointRec(mousePos, optionRect) &&
          IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        selectedIndex = static_cast<int>(i);
        selecting = false;
        if (callback)
          callback(options[selectedIndex]);
        break;
      }
    }
  }

  if (pressed) {
    hovered = false;
    selecting = !selecting;
  }
}

void ComboBox::Draw() const {
  Color color = GRAY;

  if (!enabled)
    color = DARKGRAY;
  else if (hovered)
    color = WHITE;

  DrawTexturePro(texture, trect, bounds, {0, 0}, 0.0f, color);
  DrawText(label.c_str(), static_cast<int>(bounds.x + bounds.width + 5),
           static_cast<int>(bounds.y + bounds.height / 2 - FONT_SIZE / 2),
           FONT_SIZE, WHITE);

  std::string displayText = selectedIndex >= 0 && selectedIndex < options.size()
                                ? options[selectedIndex]
                                : "Select";
  int textWidth = MeasureText(displayText.c_str(), FONT_SIZE);

  DrawText(displayText.c_str(),
           static_cast<int>(bounds.x + (bounds.width - textWidth) / 2),
           static_cast<int>(bounds.y + bounds.height / 2 - FONT_SIZE / 2),
           FONT_SIZE, WHITE);

  if (selecting) {
    for (size_t i = 0; i < options.size(); i++) {
      Rectangle optionRect = {bounds.x, bounds.y + bounds.height * (i + 1),
                              bounds.width, bounds.height};
      DrawTexturePro(texture, trect, optionRect, {0, 0}, 0.0f,
                     i == selectedIndex ? GRAY : ORANGE);
      DrawText(options[i].c_str(),
               static_cast<int>(optionRect.x +
                                (optionRect.width -
                                 MeasureText(options[i].c_str(), FONT_SIZE)) /
                                    2),
               static_cast<int>(optionRect.y + optionRect.height / 2 -
                                FONT_SIZE / 2),
               FONT_SIZE, WHITE);
    }
  }
}

Rectangle ComboBox::GetBounds() const {
  return {bounds.x, bounds.y,
          bounds.width + MeasureText(label.c_str(), FONT_SIZE) + 10,
          bounds.height * (options.size() + 1)};
}

void ComboBox::SetPosition(float x, float y) {
  bounds.x = x;
  bounds.y = y;
}

void ComboBox::SetSize(float width, float height) {
  bounds.width = width;
  bounds.height = height;
}

void ComboBox::SetText(const std::string &text) {
  if (selectedIndex >= 0 && selectedIndex < options.size())
    options[selectedIndex] = text;
}

void ComboBox::SetLabel(const std::string &text) { label = text; }

bool ComboBox::Selecting() const { return selecting; }

void ComboBox::SetOptions(const std::vector<std::string> &newOptions) {
  options = newOptions;
  if (selectedIndex >= options.size())
    selectedIndex = -1;
}

std::vector<std::string> ComboBox::GetOptions() { return options; }

void ComboBox::SetSelectedIndex(int index) {
  if (index >= 0 && index < options.size())
    selectedIndex = index;
}

int ComboBox::GetSelectedIndex() const { return selectedIndex; }

void Widget::SetEnabled(bool value) { enabled = value; }

void UIManager::Update() {
  if (!enabled)
    return;
  if (!active)
    return;
  Vector2 mousePos = GetMousePosition();
  if (!IsFlagActive(UiWindowFlags_NoMove)) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(mousePos, window)) {
      dragging = true;
      for (auto &widget : widgets) {
        Rectangle bounds = widget->GetBounds();
        bounds.x += window.x;
        bounds.y += window.y;
        if (CheckCollisionPointRec(mousePos, bounds)) {
          dragging = false;
          break;
        }
      }
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
      dragging = false;
    if (dragging) {
      Vector2 delta = GetMouseDelta();
      window.x += delta.x;
      window.y += delta.y;
    }
  }
  if (!CheckCollisionPointRec(mousePos, window))
    return;
  mousePos.x -= window.x;
  mousePos.y -= window.y;
  for (size_t i = widgets.size(); i > 0; i--) {
    auto &widget = widgets[i - 1];
    if (!widget->visible)
      continue;
    widget->Update(mousePos);
    if (dynamic_cast<ComboBox *>(widget.get()) != nullptr) {
      // check if selecting is active and break
      if (dynamic_cast<ComboBox *>(widget.get())->Selecting())
        break;
    }
  }
}

void UIManager::Draw() {
  if (!enabled)
    return;
  BeginTextureMode(windowTexture);
  ClearBackground({0, 0, 0, 0});
  if (!IsFlagActive(UiWindowFlags_NoBackground)) {
    DrawRectangleRounded({0, 0, window.width, window.height},
                         style.borderRounding, 8, style.background);
  }
  if (!IsFlagActive(UiWindowFlags_NoTitleBar))
    m_DrawTitlebar();

  for (const auto &widget : widgets) {
    if (!widget->visible)
      continue;
    widget->Draw();
  }
  EndTextureMode();
  DrawTexturePro(windowTexture.texture,
                 {0, 0, static_cast<float>(windowTexture.texture.width),
                  static_cast<float>(-windowTexture.texture.height)},
                 window, {0, 0}, 0.0f, WHITE);
  if (!IsFlagActive(UiWindowFlags_NoBackground))
    DrawRectangleRoundedLinesEx(window, style.borderRounding, 8,
                                style.borderSize, style.borderColor);
}

void UIManager::clear() { widgets.clear(); }

void UIManager::SetPosition(float x, float y) {
  window.x = x;
  window.y = y;
  logicalWindow.x = x;
  logicalWindow.y = y;
}

void UIManager::SetSize(float width, float height) {
  window.width = width;
  window.height = height;
  UnloadRenderTexture(windowTexture);
  windowTexture = LoadRenderTexture(static_cast<int>(window.width),
                                    static_cast<int>(window.height));
}

void UIManager::SetFlags(UiWindowFlags flags) { this->flags = flags; }

void UIManager::SetFlag(UiWindowFlags flag) { flags = flags | flag; }

void UIManager::ClearFlag(UiWindowFlags flag) { flags = flags & ~flag; }

bool UIManager::IsFlagActive(UiWindowFlags flag) {
  return (flags & flag) == flag;
}

void UIManager::SetName(const std::string &name) { this->name = name; }

std::string UIManager::GetName() const { return name; }

void UIManager::m_DrawTitlebar() {
  DrawRectangleRec({0, 0, window.width, style.titlebarHeight}, style.accent);
  DrawText(name.c_str(), static_cast<int>(style.spacing),
           static_cast<int>(style.titlebarHeight / 2 - style.fontSize / 2),
           style.fontSize, WHITE);
  DrawLineEx({0, style.titlebarHeight}, {window.width, style.titlebarHeight},
             style.borderSize, style.borderColor);
}

void UIManager::SetEnabled(bool value) { enabled = value; }

bool UIManager::GetEnabled() const { return enabled; }

void UIManager::SetActive(bool value) { active = value; }

bool UIManager::GetActive() const { return active; }

void WindowManager::clear() { windows.clear(); }

void WindowManager::Update() {
  if (windows.empty())
    return;
  Vector2 mousePos = GetMousePosition();
  for (size_t i = 0; i < windows.size(); i++) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        !CheckCollisionPointRec(mousePos, windows.back()->GetWindowRect()) &&
        CheckCollisionPointRec(mousePos, windows[i]->GetWindowRect())) {
      std::swap(windows[windows.size() - 1], windows[i]);
      break;
    }
  }
  windows[windows.size() - 1]->Update();
}

void WindowManager::Draw() {
  for (auto &window : windows) {
    window->Draw();
  }
}

Checkbox::Checkbox(Texture &t, Vector2 position, float size, std::string text,
                   bool *val, Callback onClick)
    : texture(t), label(text), val(val), callback(onClick) {
  assert(val != nullptr && "Invalid val pointer (nullptr)!");
  bounds.x = position.x;
  bounds.y = position.y;
  const int TEXT_SIZE = MeasureText(text.data(), static_cast<int>(size)) + 5;
  bounds.width = size + TEXT_SIZE;
  bounds.height = size;
  trect.x = 0;
  trect.y = 0;
  trect.width = static_cast<float>(t.width);
  trect.height = static_cast<float>(t.height);
}

void Checkbox::Update(const Vector2 mousePos) {
  if (!enabled)
    return;

  hovered = CheckCollisionPointRec(mousePos, bounds);
  if (!hovered)
    return;

  pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

  if (pressed) {
    hovered = false;
    *val = !*val;
    if (callback)
      callback();
  }
}

void Checkbox::Draw() const {
  Color color = GRAY;

  if (!enabled)
    color = DARKGRAY;
  else if (hovered)
    color = WHITE;

  DrawTexturePro(texture, trect,
                 {bounds.x, bounds.y, bounds.height, bounds.height}, {0, 0},
                 0.0f, color);
  if (*val) {
    // Scale rectangle relative to checkbox size
    const float padding = bounds.height * 0.25f;

    const Rectangle innerRect = {bounds.x + padding, bounds.y + padding,
                                 bounds.height - (padding * 2.0f),
                                 bounds.height - (padding * 2.0f)};

    DrawRectangleRec(innerRect, GRAY);
  }

  DrawText(label.c_str(), static_cast<int>(bounds.x + bounds.height + 5),
           static_cast<int>(bounds.y), static_cast<int>(bounds.height), WHITE);
}

Rectangle Checkbox::GetBounds() const { return bounds; }

void Checkbox::SetPosition(float x, float y) {
  bounds.x = x;
  bounds.y = y;
}

void Checkbox::SetSize(float size) {
  bounds.height = size;
  bounds.width = size + MeasureText(label.data(), static_cast<int>(size)) + 5;
}

void Checkbox::SetText(const std::string &text) {
  label = text;
  SetSize(bounds.height);
}

InputInt::InputInt(Texture &t, Rectangle bounds, std::string text, int *val,
                   int step, int stepshift, Callback onClick)
    : texture(t), bounds(bounds), label(text), val(val), step(step),
      stepshift(stepshift), callback(onClick), trect(0, 0, t.width, t.height),
      stepPlus(
          t,
          {bounds.x + bounds.width + 5, bounds.y, bounds.height, bounds.height},
          "+",
          [&]() {
            int v = *this->val;
            if (IsKeyDown(KEY_LEFT_SHIFT))
              v += this->stepshift;
            else
              v += this->step;
            *this->val = v;
          }),
      stepMinus(t,
                {bounds.x + bounds.width + bounds.height + 10, bounds.y,
                 bounds.height, bounds.height},
                "-", [&]() {
                  int v = *this->val;
                  if (IsKeyDown(KEY_LEFT_SHIFT))
                    v -= this->stepshift;
                  else
                    v -= this->step;
                  *this->val = v;
                }) {}

void InputInt::Update(const Vector2 mousePos) {
  if (!enabled)
    return;
  stepPlus.Update(mousePos);
  stepMinus.Update(mousePos);
  hovered = CheckCollisionPointRec(mousePos, bounds);
  if (!hovered && !pressed) {
    hovered = false;
    return;
  }

  if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    pressed = true;
    valStr = std::to_string(*val);
    if (callback)
      callback();
  } else if ((!hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ||
             IsKeyPressed(KEY_ENTER)) {
    pressed = false;
    if (valStr.empty())
      valStr = "0";
    else if (valStr == "-")
      valStr = "0";
    *val = std::stoi(valStr);
  } else if (IsKeyPressed(KEY_ESCAPE)) {
    pressed = false;
    valStr = std::to_string(*val);
  }

  if (pressed) {
    const unsigned int key = GetKeyPressed();
    if ((key >= '0' && key <= '9') || key == '+') {
      valStr += static_cast<char>(key);
    } else if (valStr.empty() && key == 47) {
      valStr += '-';
    } else if (key == KEY_BACKSPACE && !valStr.empty()) {
      valStr.pop_back();
    }
  }
}

void InputInt::Draw() const {
  stepPlus.Draw();
  stepMinus.Draw();
  Color color = GRAY;

  if (!enabled)
    color = DARKGRAY;
  else if (hovered)
    color = WHITE;
  if (pressed)
    color = RED;

  DrawTexturePro(texture, trect, bounds, {0, 0}, 0.0f, color);
  std::string valstr;
  if (!pressed)
    valstr = std::to_string(*val);
  else
    valstr = valStr;
  int textWidth = MeasureText(valstr.c_str(), FONT_SIZE);

  DrawText(valstr.c_str(),
           static_cast<int>(bounds.x + (bounds.width - textWidth) / 2),
           static_cast<int>(bounds.y + bounds.height / 2 - FONT_SIZE / 2),
           FONT_SIZE, WHITE);
  const Rectangle b = GetBounds();
  DrawText(
      label.c_str(),
      static_cast<int>(b.x + b.width -
                       MeasureText(label.data(), static_cast<int>(FONT_SIZE))),
      static_cast<int>(b.y + bounds.height / 2 - FONT_SIZE / 2), FONT_SIZE,
      WHITE);
}

Rectangle InputInt::GetBounds() const {
  Rectangle spb = stepPlus.GetBounds();
  Rectangle smb = stepMinus.GetBounds();
  return {bounds.x, bounds.y,
          bounds.width + 10 + spb.width + smb.width + 5 +
              MeasureText(label.data(), static_cast<int>(FONT_SIZE))};
}

void InputInt::SetPosition(float x, float y) {
  bounds.x = x;
  bounds.y = y;
  stepPlus.SetPosition(bounds.x + bounds.width + 5, bounds.y);
  stepMinus.SetPosition(bounds.x + bounds.width + bounds.height + 10, bounds.y);
}

void InputInt::SetSize(float width, float height) {
  bounds.width = width;
  bounds.height = height;
  stepPlus.SetPosition(bounds.x + bounds.width + 5, bounds.y);
  stepPlus.SetSize(bounds.height, bounds.height);
  stepMinus.SetPosition(bounds.x + bounds.width + bounds.height + 10, bounds.y);
  stepMinus.SetSize(bounds.height, bounds.height);
}

void InputInt::SetText(const std::string &text) { label = text; }

InputDouble::InputDouble(Texture &t, Rectangle bounds, std::string text,
                         double *val, double step, double stepshift,
                         Callback onClick)
    : texture(t), bounds(bounds), label(text), val(val), step(step),
      stepshift(stepshift), callback(onClick), trect(0, 0, t.width, t.height),
      stepPlus(
          t,
          {bounds.x + bounds.width + 5, bounds.y, bounds.height, bounds.height},
          "+",
          [&]() {
            double v = *this->val;
            if (IsKeyDown(KEY_LEFT_SHIFT))
              v += this->stepshift;
            else
              v += this->step;
            *this->val = v;
          }),
      stepMinus(t,
                {bounds.x + bounds.width + bounds.height + 10, bounds.y,
                 bounds.height, bounds.height},
                "-", [&]() {
                  double v = *this->val;
                  if (IsKeyDown(KEY_LEFT_SHIFT))
                    v -= this->stepshift;
                  else
                    v -= this->step;
                  *this->val = v;
                }) {}

void InputDouble::Update(const Vector2 mousePos) {
  if (!enabled)
    return;
  stepPlus.Update(mousePos);
  stepMinus.Update(mousePos);
  hovered = CheckCollisionPointRec(mousePos, bounds);
  if (!hovered && !pressed) {
    hovered = false;
    return;
  }

  if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    pressed = true;
    valStr = FormatValue();
    if (callback)
      callback();
  } else if ((!hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ||
             IsKeyPressed(KEY_ENTER)) {
    pressed = false;
    if (valStr.empty())
      valStr = "0";
    else if (valStr == "-" || valStr == ".")
      valStr = "0";
    *val = std::stod(valStr);
  } else if (IsKeyPressed(KEY_ESCAPE)) {
    pressed = false;
    valStr = FormatValue();
  }

  if (pressed) {
    const unsigned int key = GetKeyPressed();
    if ((key >= '0' && key <= '9') || key == '+' || key == '.') {
      valStr += static_cast<char>(key);
    } else if (valStr.empty() && key == 47) {
      valStr += '-';
    } else if (key == KEY_BACKSPACE && !valStr.empty()) {
      valStr.pop_back();
    }
  }
}

void InputDouble::Draw() const {
  stepPlus.Draw();
  stepMinus.Draw();
  Color color = GRAY;

  if (!enabled)
    color = DARKGRAY;
  else if (hovered)
    color = WHITE;
  if (pressed)
    color = RED;

  DrawTexturePro(texture, trect, bounds, {0, 0}, 0.0f, color);
  std::string valstr;
  if (!pressed)
    valstr = FormatValue();
  else
    valstr = valStr;
  int textWidth = MeasureText(valstr.c_str(), FONT_SIZE);

  DrawText(valstr.c_str(),
           static_cast<int>(bounds.x + (bounds.width - textWidth) / 2),
           static_cast<int>(bounds.y + bounds.height / 2 - FONT_SIZE / 2),
           FONT_SIZE, WHITE);
  const Rectangle b = GetBounds();
  DrawText(
      label.c_str(),
      static_cast<int>(b.x + b.width -
                       MeasureText(label.data(), static_cast<int>(FONT_SIZE))),
      static_cast<int>(b.y + bounds.height / 2 - FONT_SIZE / 2), FONT_SIZE,
      WHITE);
}

Rectangle InputDouble::GetBounds() const {
  Rectangle spb = stepPlus.GetBounds();
  Rectangle smb = stepMinus.GetBounds();
  return {bounds.x, bounds.y,
          bounds.width + 10 + spb.width + smb.width + 5 +
              MeasureText(label.data(), static_cast<int>(FONT_SIZE))};
}

void InputDouble::SetPosition(float x, float y) {
  bounds.x = x;
  bounds.y = y;
  stepPlus.SetPosition(bounds.x + bounds.width + 5, bounds.y);
  stepMinus.SetPosition(bounds.x + bounds.width + bounds.height + 10, bounds.y);
}

void InputDouble::SetSize(float width, float height) {
  bounds.width = width;
  bounds.height = height;
  stepPlus.SetPosition(bounds.x + bounds.width + 5, bounds.y);
  stepPlus.SetSize(bounds.height, bounds.height);
  stepMinus.SetPosition(bounds.x + bounds.width + bounds.height + 10, bounds.y);
  stepMinus.SetSize(bounds.height, bounds.height);
}

void InputDouble::SetText(const std::string &text) { label = text; }

std::string InputDouble::FormatValue() const {
  char buffer[64];
  std::snprintf(buffer, sizeof(buffer), formatStr.c_str(), *val);
  return std::string(buffer);
}

InputString::InputString(Texture &t, Rectangle bounds, std::string text,
                         std::string *val, size_t limit, Callback onClick)
    : texture(t), bounds(bounds), label(text), val(val), limit(limit),
      callback(onClick), trect(0, 0, t.width, t.height) {}

void InputString::Update(const Vector2 mousePos) {
  if (!enabled)
    return;

  hovered = CheckCollisionPointRec(mousePos, bounds);
  if (!hovered && !pressed) {
    hovered = false;
    return;
  }

  if (hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    pressed = true;
    inputStr = *val;
    if (callback)
      callback();
  } else if ((!hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ||
             IsKeyPressed(KEY_ENTER)) {
    pressed = false;
    *val = inputStr;
  } else if (IsKeyPressed(KEY_ESCAPE)) {
    pressed = false;
    inputStr = *val;
  }

  if (pressed) {
    const unsigned int key = GetKeyPressed();
    if (inputStr.length() < limit &&
        (key >= 32 && key <= 126)) { // Printable characters
      inputStr += static_cast<char>(key);
    } else if (key == KEY_BACKSPACE && !inputStr.empty()) {
      inputStr.pop_back();
    }
  }
}

void InputString::Draw() const {
  Color color = GRAY;

  if (!enabled)
    color = DARKGRAY;
  else if (hovered)
    color = WHITE;
  if (pressed)
    color = RED;

  DrawTexturePro(texture, trect, bounds, {0, 0}, 0.0f, color);
  std::string displayStr = pressed ? inputStr : *val;
  int textWidth = MeasureText(displayStr.c_str(), FONT_SIZE);

  DrawText(displayStr.c_str(),
           static_cast<int>(bounds.x + (bounds.width - textWidth) / 2),
           static_cast<int>(bounds.y + bounds.height / 2 - FONT_SIZE / 2),
           FONT_SIZE, WHITE);
  const Rectangle b = GetBounds();
  DrawText(
      label.c_str(),
      static_cast<int>(b.x + b.width -
                       MeasureText(label.data(), static_cast<int>(FONT_SIZE))),
      static_cast<int>(b.y + bounds.height / 2 - FONT_SIZE / 2), FONT_SIZE,
      WHITE);
}

Rectangle InputString::GetBounds() const {
  return {bounds.x, bounds.y,
          bounds.width + 5 +
              MeasureText(label.data(), static_cast<int>(FONT_SIZE)),
          bounds.height};
}

void InputString::SetPosition(float x, float y) {
  bounds.x = x;
  bounds.y = y;
}

void InputString::SetSize(float width, float height) {
  bounds.width = width;
  bounds.height = height;
}

void InputString::SetText(const std::string &text) { label = text; }