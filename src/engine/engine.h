#pragma once

#include "UUID.h"
#include <cstdint>
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <raylib.h>
#include <functional>
#include <utility>
#include <memory>

using json = nlohmann::json;

// Predefinitions
class Entity;

/*
ComponentRegistration:
  - give it a name, serialization and deserialization function
  - storing and managing engine available components
*/
struct ComponentRegistration {
  std::string name;

  std::function<bool(Entity)> HasComponent;
  std::function<void(Entity)> AddComponent;
  std::function<void(Entity)> RemoveComponent;
  std::function<void(Entity, json&)> Serialize;
  std::function<void(Entity, const json&)> Deserialize;
};

class ComponentRegistry {
public:
  static ComponentRegistry& Get();
  const std::unordered_map<std::string, ComponentRegistration>&
  GetComponents() const;
  const ComponentRegistration* FindByName(const std::string& name);

  template <typename T>
  void RegisterComponent(const std::string& name, auto serializeFn,
                         auto deserializeFn);

private:
  std::unordered_map<std::string, ComponentRegistration> m_components;
};

/*
Scene:
  - holds world registry
  - entity map for fast lookup
  - if it is a 2D scene or 3D
  - Creating new entities
  - Setting hirarchy with parents and childs
*/
class Scene {
public:
  Entity CreateEntity();
  Entity CreateEntity(const std::string& name);
  Entity CreateEntity(UUID uuid, const std::string& name);
  void DestroyEntity(Entity entity);
  void DestroyEntity(UUID uuid);
  void DestroyEntity(entt::entity entity);

  Entity GetEntityByUUID(UUID id);

  void SetParent(Entity child, Entity parent);
  void RemoveParent(Entity child);
  std::vector<Entity> GetRootEntities();

#ifdef MENGLED_DEV
  std::map<uint64_t, entt::entity>* GetMap() { return &m_entityMap; }
#else
  std::unordered_map<uint64_t, entt::entity>* GetMap() {
    return &m_entityMap;
  }
#endif

  Matrix GetWorldTransform(Entity entity);

  void Update(float dt);
  void Clear();

private:
#ifdef MENGLED_DEV
  std::map<uint64_t, entt::entity> m_entityMap;
#else
  std::unordered_map<uint64_t, entt::entity> m_entityMap;
#endif
  bool m_twoD = false;

public:
  entt::registry registry;
};

/*
Entity:
  - Holds the scene it belongs to
  - adding components
  - getting components
  - check if contains component
*/
class Entity {
public:
  Entity() = default;
  Entity(entt::entity handle, Scene* scene);

  template <typename T, typename... Args> T& AddComponent(Args&&... args) {
    return m_scene->registry.emplace<T>(m_handle,
                                        std::forward<Args>(args)...);
  }

  template <typename T> T& GetComponent() {
    return m_scene->registry.get<T>(m_handle);
  }
  template <typename T> T* TryGetComponent() {
    return m_scene->registry.get<T>(m_handle);
  }

  template <typename T> bool HasComponent() const {
    return m_scene->registry.all_of<T>(m_handle);
  }
  template <typename T> void RemoveComponent() const {
    m_scene->registry.remove<T>(m_handle);
  }

  operator bool() const { return m_handle != entt::null; }

private:
  entt::entity m_handle = entt::null;
  Scene* m_scene = nullptr;
};

/*
SceneManager:
  - Scene loading
  - Scene saving
  - Changing scenes
*/
class SceneManager {
private:
  std::unique_ptr<Scene> m_currentScene;

public:
  void LoadScene(const std::string& path);
  Scene* GetCurrentScene();
};

/*
GlobalGameState:
  - Used later on
  - Set triggers to evaluate each frame
*/
class GlobalGameState {};

/*
SceneSerializer:
  - load single scene
  - save single scene
*/
class SceneSerializer {
public:
  SceneSerializer(Scene* scene);

  bool Serialize(const std::string& filepath);
  bool Deserialize(const std::string& filepath);

private:
  Scene* m_scene;
};

class Engine {
public:
  void init();

private:
  void RegisterEngineComponents();

public:
  SceneManager sceneManager;
  GlobalGameState globalState;
};

void TestEngine();