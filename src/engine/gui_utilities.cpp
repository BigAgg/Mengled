#include "gui_utilities.h"

#ifndef FONT_SIZE
#define FONT_SIZE 20
#endif

#include <imgui.h>
#include "utils/logging.h"

Button::Button(Texture &t, Rectangle rect, std::string text, Callback onClick)
: texture(t), bounds(rect), label(std::move(text)), callback(std::move(onClick))
{
	trect.x = 0;
	trect.y = 0;
	trect.width = static_cast<float>(t.width);
	trect.height = static_cast<float>(t.height);
}

void Button::Update() {
	if (!enabled)
		return;

	Vector2 mouse = GetMousePosition();
	hovered = CheckCollisionPointRec(mouse, bounds);
	if (!hovered)
		return;

	pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

	if (pressed && callback)
		callback();
}

void Button::Draw() const {
	Color color = GRAY;

	if (!enabled)
		color = DARKGRAY;
	else if (hovered)
		color = WHITE;

	DrawTexturePro(texture, trect, bounds, { 0, 0 }, 0.0f, color);

	int textWidth = MeasureText(label.c_str(), FONT_SIZE);

	DrawText(label.c_str(),
		static_cast<int>(bounds.x + (bounds.width - textWidth) / 2),
		static_cast<int>(bounds.y + bounds.height / 2 - FONT_SIZE / 2),
		FONT_SIZE,
		WHITE);
}

Rectangle Button::GetBounds() const {
	return bounds;
}

void Button::SetPosition(float x, float y){
	bounds.x = x;
	bounds.y = y;
}

void Button::SetSize(float width, float height) {
	bounds.width = width;
	bounds.height = height;
}

void Button::SetText(const std::string& text) {
	label = text;
}

void Widget::SetEnabled(bool value) {
	enabled = value;
}

void UIManager::Update() {
	for (auto& widget : widgets) {
		if (!widget->visible)
			continue;
		widget->Update();
	}
}

void UIManager::Draw() {
	for (const auto& widget : widgets) {
		if (!widget->visible)
			continue;
		widget->Draw();
	}
}

void UIManager::clear() {
	widgets.clear();
}

void UIManager::SetFlags(UiWindowFlags flags) {
	this->flags = flags;
}

void UIManager::SetFlag(UiWindowFlags flag) {
	flags = flags | flag;
}

void UIManager::ClearFlag(UiWindowFlags flag) {
	flags = flags & flag;
}

bool UIManager::IsFlagActive(UiWindowFlags flag) {
	return flags & flag;
}

Checkbox::Checkbox(Texture& t, Vector2 position, float size, std::string text, bool* val, Callback onClick)
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

void Checkbox::Update() {
	if (!enabled)
		return;

	Vector2 mouse = GetMousePosition();
	hovered = CheckCollisionPointRec(mouse, bounds);
	if (!hovered)
		return;

	pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

	if (pressed) {
		*val = !*val;
		if(callback)
			callback();
	}
}

void Checkbox::Draw() const {
	Color color = GRAY;

	if (!enabled)
		color = DARKGRAY;
	else if (hovered)
		color = WHITE;

	DrawTexturePro(texture, trect, {bounds.x, bounds.y, bounds.height, bounds.height}, {0, 0}, 0.0f, color);
	if (*val) {
		// Scale rectangle relative to checkbox size
		const float padding = bounds.height * 0.25f;

		const Rectangle innerRect = {
			bounds.x + padding,
			bounds.y + padding,
			bounds.height - (padding * 2.0f),
			bounds.height - (padding * 2.0f)
		};

		DrawRectangleRec(innerRect, GRAY);
	}

	DrawText(label.c_str(),
		static_cast<int>(bounds.x + bounds.height + 5),
		static_cast<int>(bounds.y),
		static_cast<int>(bounds.height),
		WHITE);
}

Rectangle Checkbox::GetBounds() const {
	return bounds;
}

void Checkbox::SetPosition(float x, float y) {
	bounds.x = x;
	bounds.y = y;
}

void Checkbox::SetSize(float size) {
	bounds.height = size;
	bounds.width = size + MeasureText(label.data(), static_cast<int>(size)) + 5;
}

void Checkbox::SetText(const std::string& text) {
	label = text;
	SetSize(bounds.height);
}

InputInt::InputInt(Texture& t, Rectangle bounds, std::string text, int* val, int step, int stepshift, Callback onClick)
	: texture(t), bounds(bounds), label(text), val(val), step(step), stepshift(stepshift), callback(onClick), trect(0, 0, t.width, t.height),
	stepPlus(t, { bounds.x + bounds.width + 5, bounds.y, bounds.height, bounds.height }, "+", [&]() {
			int v = *this->val;
			if (IsKeyDown(KEY_LEFT_SHIFT))
				v += this->stepshift;
			else
				v+=this->step;
			*this->val = v;
		}),
	stepMinus(t, { bounds.x + bounds.width + bounds.height + 10, bounds.y, bounds.height, bounds.height }, "-", [&]() {
			int v = *this->val;
			if (IsKeyDown(KEY_LEFT_SHIFT))
				v -= this->stepshift;
			else
				v-=this->step;
			*this->val = v;
		})
{}

void InputInt::Update() {
	if (!enabled)
		return;
	stepPlus.Update();
	stepMinus.Update();
	Vector2 mouse = GetMousePosition();
	hovered = CheckCollisionPointRec(mouse, bounds);
	if (!hovered)
		return;

	pressed = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

	if (pressed) {
		if(callback)
			callback();
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

	DrawTexturePro(texture, trect, bounds, { 0, 0 }, 0.0f, color);
	
	const std::string valstr = std::to_string(*val);
	int textWidth = MeasureText(valstr.c_str(), FONT_SIZE);

	DrawText(valstr.c_str(),
		static_cast<int>(bounds.x + (bounds.width - textWidth) / 2),
		static_cast<int>(bounds.y + bounds.height / 2 - FONT_SIZE / 2),
		FONT_SIZE,
		WHITE);
	const Rectangle b = GetBounds();
	DrawText(label.c_str(),
		static_cast<int>(b.x + b.width - MeasureText(label.data(), static_cast<int>(FONT_SIZE))),
		static_cast<int>(b.y + bounds.height / 2 - FONT_SIZE / 2),
		FONT_SIZE,
		WHITE);
}

Rectangle InputInt::GetBounds() const {
	Rectangle spb = stepPlus.GetBounds();
	Rectangle smb = stepMinus.GetBounds();
	return { bounds.x, bounds.y, bounds.width + 10 + spb.width + smb.width +
		5 + MeasureText(label.data(), static_cast<int>(FONT_SIZE))};
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

void InputInt::SetText(const std::string & text) {
	label = text;
} 