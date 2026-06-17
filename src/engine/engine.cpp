#include "engine.h"

#include "components.h"
#include "json_serializer.h"
#include "raymath.h"
#include "utils/logging.h"
#include <cstdint>
#include <fstream>

using json = nlohmann::json;

ComponentRegistry& ComponentRegistry::Get() {
  static ComponentRegistry instance;
  return instance;
}

const std::unordered_map<std::string, ComponentRegistration>&
ComponentRegistry::GetComponents() const {
  return m_components;
}

const ComponentRegistration* ComponentRegistry::FindByName(const std::string& name) {
  return &m_components[name];
  auto it = m_components.find(name);
  if (it == m_components.end())
    return nullptr;
  return &it->second;
}

template <typename T>
void ComponentRegistry::RegisterComponent(const std::string& name, auto serializeFn,
                                          auto deserializeFn) {
  ComponentRegistration reg;
  reg.name = name;
  reg.HasComponent = [](Entity entity) { return entity.HasComponent<T>(); };
  reg.AddComponent = [](Entity entity) {
    if (!entity.HasComponent<T>())
      entity.AddComponent<T>();
  };
  reg.RemoveComponent = [](Entity entity) {
    if (entity.HasComponent<T>())
      entity.RemoveComponent<T>();
  };
  reg.Serialize = [serializeFn, name](Entity entity, json& json) {
    if (entity.HasComponent<T>()) {
      json[name] = serializeFn(entity.GetComponent<T>());
    }
  };
  reg.Deserialize = [deserializeFn, name](Entity entity, const nlohmann::json& json) {
    if (!json.contains(name))
      return;

    if (!entity.HasComponent<T>())
      entity.AddComponent<T>();

    deserializeFn(entity.GetComponent<T>(), json[name]);
  };
  m_components[name] = std::move(reg);
}

Entity::Entity(entt::entity handle, Scene* scene) : m_handle(handle), m_scene(scene) {}

Entity Scene::CreateEntity() { return CreateEntity("Entity"); }

Entity Scene::CreateEntity(const std::string& name) {
  UUID uuid;

  Entity entity = CreateEntity(uuid, name);

  return entity;
}

Entity Scene::CreateEntity(UUID uuid, const std::string& name) {
  auto handle = registry.create();
  Entity entity(handle, this);

  entity.AddComponent<IDComponent>(uuid);
  entity.AddComponent<NameComponent>(name);
  entity.AddComponent<RelationshipComponent>();

  m_entityMap[(uint64_t)uuid] = handle;

  return entity;
}

void Scene::DestroyEntity(Entity entity) {
  if (!entity)
    return;
  if (!entity.HasComponent<IDComponent>())
    return;
  UUID uuid = entity.GetComponent<IDComponent>().ID;
  DestroyEntity(uuid);
}
void Scene::DestroyEntity(UUID uuid) {
  auto it = m_entityMap.find((uint64_t)uuid);
  if (it == m_entityMap.end())
    return;
  DestroyEntity(m_entityMap[(uint64_t)uuid]);
  m_entityMap.erase(it);
}
void Scene::DestroyEntity(entt::entity entity) {
  auto c = registry.try_get<RelationshipComponent>(entity);
  if (c) {
    if (c->parent)
      RemoveParent((Entity(entity, this)));
    for (const auto& child : c->children) {
      DestroyEntity(child);
    }
  }
  registry.destroy(entity);
}

Entity Scene::GetEntityByUUID(UUID id) {
  auto it = m_entityMap.find((uint64_t)id);

  if (it == m_entityMap.end())
    return {};

  return Entity(it->second, this);
}

void Scene::SetParent(Entity child, Entity parent) {
  auto& childRel = child.GetComponent<RelationshipComponent>();
  auto& childID = child.GetComponent<IDComponent>().ID;

  if (childRel.parent.IsValid()) {
    Entity oldParent = GetEntityByUUID(childRel.parent);

    if (oldParent) {
      auto& oldRel = oldParent.GetComponent<RelationshipComponent>();
      auto& children = oldRel.children;

      children.erase(std::remove(children.begin(), children.end(), childID), children.end());
    }
  }

  auto& parentID = parent.GetComponent<IDComponent>().ID;

  childRel.parent = parentID;
  parent.GetComponent<RelationshipComponent>().children.push_back(childID);
}

void Scene::RemoveParent(Entity child) {
  auto& childRel = child.GetComponent<RelationshipComponent>();

  if (!childRel.parent.IsValid())
    return;

  Entity parent = GetEntityByUUID(childRel.parent);

  if (parent) {
    auto& children = parent.GetComponent<RelationshipComponent>().children;
    auto childID = child.GetComponent<IDComponent>().ID;

    children.erase(std::remove(children.begin(), children.end(), childID), children.end());
  }

  childRel.parent = 0;
}

std::vector<Entity> Scene::GetRootEntities() {
  std::vector<Entity> result;

  auto view = registry.view<IDComponent, RelationshipComponent>();

  for (auto entityHandle : view) {
    auto& rel = view.get<RelationshipComponent>(entityHandle);

    if (!rel.parent.IsValid()) {
      result.emplace_back(entityHandle, this);
    }
  }
  return result;
}

void Scene::Update(float dt) {}

void Scene::Clear() {
  m_twoD = false;
  m_entityMap.clear();
  registry.clear();
}

Matrix Scene::GetWorldTransform(Entity entity) {
  auto& transform = entity.GetComponent<Transform3D>();

  Matrix local = MatrixMultiply(
      MatrixMultiply(
          MatrixScale(transform.scale.x, transform.scale.y, transform.scale.z),
          MatrixRotateXYZ({DEG2RAD * transform.rotation.x, DEG2RAD * transform.rotation.y,
                           DEG2RAD * transform.rotation.z})),
      MatrixTranslate(transform.position.x, transform.position.y, transform.position.z));

  auto& rel = entity.GetComponent<RelationshipComponent>();

  if (!rel.parent.IsValid())
    return local;

  Entity parent = GetEntityByUUID(rel.parent);

  return MatrixMultiply(local, GetWorldTransform(parent));
}

void SceneManager::LoadScene(const std::string& path) {
  m_currentScene = std::make_unique<Scene>();
}

Scene* SceneManager::GetCurrentScene() { return m_currentScene.get(); }

SceneSerializer::SceneSerializer(Scene* scene) : m_scene(scene) {}
bool SceneSerializer::Serialize(const std::string& filepath) {
  json root;
  root["Scene"] = "Untitled";
  auto& registry = ComponentRegistry::Get();
  auto& entities = root["Entities"];
  entities = json::array();
  auto view = m_scene->registry.view<IDComponent>();
  for (auto entityHandle : view) {
    json entityJson;
    Entity entity(entityHandle, m_scene);
    for (auto& component : registry.GetComponents()) {
      component.second.Serialize(entity, entityJson);
    }
    entities.push_back(entityJson);
  }
  // write file
  std::ofstream file;
  file.open(filepath, std::ios::binary);
  if (!file)
    return false;
  file << root.dump(4);
  file.close();
  return true;
}

bool SceneSerializer::Deserialize(const std::string& filepath) {
  std::ifstream file;
  file.open(filepath, std::ios::binary);
  if (!file) {
    logging::logwarning("[SceneSerializer::Deserialize] Unable to open file: %s", filepath);
    return false;
  }
  logging::loginfo("[SceneSerializer::Deserialize] Loading scene from file: %s", filepath);
  json root;
  file >> root;
  file.close();
  m_scene->Clear();
  // Create entites
  logging::loginfo("Deserialize Entities...");
  for (auto& entityJson : root["Entities"]) {
    logging::loginfo("Deserializing entity...");
    UUID uuid = (uint64_t)entityJson["UUID"]["UUID"];
    std::string name = entityJson["Name"]["Name"];
    logging::loginfo("Name: %s UUID: %s", name.c_str(), std::to_string(uuid));

    Entity entity = m_scene->CreateEntity(uuid, name);
    for (auto& component : ComponentRegistry::Get().GetComponents()) {
      if (component.second.HasComponent(entity)) {
        logging::loginfo("Deserializing component: %s", component.first.c_str());
        component.second.Deserialize(entity, entityJson);
      }
    }
  }
  logging::loginfo("Entities Deserialized!");
  // Recreate relationships
  for (auto& entityJson : root["Entities"]) {
    logging::loginfo("Building Relationship for next entity...");
    UUID uuid = (uint64_t)entityJson["UUID"]["UUID"];
    Entity entity = m_scene->GetEntityByUUID(uuid);
    logging::loginfo("UUID: %s", std::to_string(uuid).c_str());

    if (!entity)
      continue;

    UUID parentID = (uint64_t)entityJson["Relationship"]["Parent"];
    logging::loginfo("Parent: %s Child: %s", std::to_string(parentID).c_str(),
                     std::to_string(uuid).c_str());
    if (parentID == 0)
      continue;
    Entity parent = m_scene->GetEntityByUUID(parentID);
    if (!parent)
      continue;
    m_scene->SetParent(entity, parent);
  }
  logging::loginfo("[SceneSerializer::Deserialize] Scene loaded from file: %s", filepath);
  return true;
}

void Engine::init() {
  RegisterEngineComponents();
  sceneManager.LoadScene("");
}

void Engine::RegisterEngineComponents() {
  auto& registry = ComponentRegistry::Get();
  registry.RegisterComponent<Transform3D>("Transform3D", SerializeTransform3D,
                                          DeserializeTransform3D);
  registry.RegisterComponent<Transform2D>("Transform2D", SerializeTransform2D,
                                          DeserializeTransform2D);
  registry.RegisterComponent<TagComponent>("Tag", SerializeTag, DeserializeTag);
  registry.RegisterComponent<IDComponent>("UUID", SerializeID, DeserializeID);
  registry.RegisterComponent<NameComponent>("Name", SerializeName, DeserializeName);
  registry.RegisterComponent<RelationshipComponent>("Relationship", SerializeRelationship,
                                                    DeserializeRelationship);
  registry.RegisterComponent<Gravity>("Gravity", SerializeGravity, DeserializeGravity);
  registry.RegisterComponent<RigidBody>("RigidBody", SerializeRigidBody, DeserializeRigidBody);
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