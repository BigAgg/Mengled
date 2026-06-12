#pragma once

#include <cstdint>
#include <string>
#include <entt/entt.hpp>
#include <unordered_map>
#include "components.h"

class Entity;

class Scene{
  public:
  Scene() = default;
  Scene(bool twoD);

  Entity CreateEntity();
  Entity CreateEntity(const std::string& name);

  Entity GetEntityByUUID(UUID id);

  void SetParent(Entity child, Entity parent);
  void RemoveParent(Entity child);
  std::vector<Entity> GetRootEntities();

  void Update(float dt);

  private:
  std::unordered_map<uint64_t, entt::entity> m_entityMap;
  bool m_twoD = false;
  
  public:
  entt::registry registry;
};

class Entity{
  public:
  Entity() = default;
  Entity(entt::entity handle, Scene* scene);

  template<typename T, typename... Args>
  T& AddComponent(Args&&... args){
    return m_scene->registry.emplace<T>(m_handle, std::forward<Args>(args)...);
  }

  template<typename T>
  T& GetComponent(){
    return m_scene->registry.get<T>(m_handle);
  }

  template<typename T>
  bool HasComponent() const{
    return m_scene->registry.all_of<T>(m_handle);
  }

  operator bool() const{
    return m_handle != entt::null;
  }

  private:
  entt::entity m_handle = entt::null;
  Scene* m_scene = nullptr;
};

class SceneManager{
  private:
  std::unique_ptr<Scene> m_currentScene;
  public:
  void LoadScene(const std::string& path);
  Scene* GetCurrentScene();
};

class GlobalGameState{
};

class SceneSerializer{
  public:
  static void Save(Scene& scene, const std::string& path);
  static void Load(Scene& scene, const std::string& path);
};

class Engine{
  public:
  SceneManager Scenes;
  GlobalGameState GlobalState;
};

void TestEngine();