#pragma once
#include "UUID.h"
#include <cstdint>
#include <raylib.h>
#include <string>
#include <vector>

// Base components
struct IDComponent {
  UUID ID;

  IDComponent() = default;
  IDComponent(UUID id) : ID(id) {}
};

struct TagComponent {
  std::string tag;

  TagComponent() = default;
  TagComponent(const std::string& tag) : tag(tag) {}
};

struct NameComponent {
  std::string name;

  NameComponent() = default;
  NameComponent(const std::string& name) : name(name) {}
};

struct RelationshipComponent {
  UUID parent = 0;
  std::vector<UUID> children;
};

// Object structs
struct TransformComponent2D {
  Vector2 position = {0.0f, 0.0f};
  float rotation = 0.0f;
  Vector2 scale = {1.0f, 1.0f};
};

struct TransformComponent3D {
  Vector3 position = {0.0f, 0.0f, 0.0f};
  Vector3 rotation = {0.0f, 0.0f, 0.0f};
  Vector3 scale = {1.0f, 1.0f, 1.0f};
};

struct VelocityComponent {
  float vel;
};

struct SpeedComponent {
  float speed;
};

// Request system
struct ChangeSceneRequest {
  std::string scenePath;
};