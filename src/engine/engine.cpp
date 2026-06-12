#include "engine.h"
#include "components.h"

Entity::Entity(entt::entity handle, Scene *scene)
    : m_handle(handle), m_scene(scene) {}

Entity Scene::CreateEntity() { return CreateEntity("Entity"); }

Entity Scene::CreateEntity(const std::string &name) {
  auto entityHandle = registry.create();

  Entity entity(entityHandle, this);

  UUID uuid;

  entity.AddComponent<IDComponent>(uuid);
  entity.AddComponent<NameComponent>(name);

  if (m_twoD)
    entity.AddComponent<TransformComponent2D>();
  else
    entity.AddComponent<TransformComponent3D>();
  entity.AddComponent<RelationshipComponent>();

  m_entityMap[(uint64_t)uuid] = entityHandle;

  return entity;
}

Entity Scene::GetEntityByUUID(UUID id) {
  auto it = m_entityMap.find((uint64_t)id);

  if (it == m_entityMap.end())
    return {};

  return Entity(it->second, this);
}

void Scene::SetParent(Entity child, Entity parent) {
  auto &childRel = child.GetComponent<RelationshipComponent>();
  auto &childID = child.GetComponent<IDComponent>().ID;

  if (childRel.parent.IsValid()) {
    Entity oldParent = GetEntityByUUID(childRel.parent);

    if (oldParent) {
      auto &oldRel = oldParent.GetComponent<RelationshipComponent>();
      auto children = oldRel.children;

      children.erase(std::remove(children.begin(), children.end(), childID),
                     children.end());
    }
  }

  auto &parentID = parent.GetComponent<IDComponent>().ID;

  childRel.parent = parentID;
  parent.GetComponent<RelationshipComponent>().children.push_back(childID);
}

void Scene::RemoveParent(Entity child) {
  auto &childRel = child.GetComponent<RelationshipComponent>();

  if (!childRel.parent.IsValid())
    return;

  Entity parent = GetEntityByUUID(childRel.parent);

  if (parent) {
    auto &children = parent.GetComponent<RelationshipComponent>().children;
    auto childID = child.GetComponent<IDComponent>().ID;

    children.erase(std::remove(children.begin(), children.end(), childID),
                   children.end());
  }

  childRel.parent = 0;
}

std::vector<Entity> Scene::GetRootEntities(){
  std::vector<Entity> result;

  auto view = registry.view<IDComponent, RelationshipComponent>();

  for(auto entityHandle : view){
    auto& rel = view.get<RelationshipComponent>(entityHandle);

    if(!rel.parent.IsValid()){
      result.emplace_back(entityHandle, this);
    }
  }
  return result;
}

void SceneManager::LoadScene(const std::string& path){
  m_currentScene = std::make_unique<Scene>();
}

Scene* SceneManager::GetCurrentScene(){
  return m_currentScene.get();
}

// Tests
void TestEntityCreation();

void TestEngine() { TestEntityCreation(); }

void TestEntityCreation() {
  Scene scene;
  auto player = scene.CreateEntity("Player");
  auto enemy = scene.CreateEntity("Enemy");

  auto playerID = player.GetComponent<IDComponent>().ID;
  auto found = scene.GetEntityByUUID(playerID);

  assert(found.HasComponent<NameComponent>());
  assert(enemy.HasComponent<IDComponent>());
}