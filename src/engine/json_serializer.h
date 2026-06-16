#pragma once

#include "components.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Vector2
static json SerializeVector2(const Vector2& v) { return {v.x, v.y}; }
static Vector2 DeserializeVector2(const json& json) { return {json[0], json[1]}; }
// Vector2
static json SerializeVector3(const Vector3& v) { return {v.x, v.y, v.z}; }
static Vector3 DeserializeVector3(const json& json) { return {json[0], json[1], json[2]}; }
// Vector4
static json SerializeVector4(const Vector4& v) { return {v.x, v.y, v.z, v.w}; }
static Vector4 DeserializeVector4(const json& json) { return {json[0], json[1], json[2], json[3]}; }
// Camera2D
static json SerializeCamera2D(const Camera2D& c) {
  return {{"Offset", SerializeVector2(c.offset)},
          {"Target", SerializeVector2(c.target)},
          {"Rotation", c.rotation},
          {"Zoom", c.zoom}};
}
static void DeserializeCamera2D(Camera2D& c, const json& json) {
  auto o = json["Offset"];
  c.offset = DeserializeVector2(o);

  auto t = json["Target"];
  c.target = DeserializeVector2(t);

  c.rotation = json["Rotation"];

  c.zoom = json["Zoom"];
}
// Camera3D
static json SerializeCamera3D(const Camera3D& c) {
  return {{"Position", SerializeVector3(c.position)},
          {"Target", SerializeVector3(c.target)},
          {"fovy", c.fovy},
          {"Projection", c.projection},
          {"up", SerializeVector3(c.up)}};
}
static void DeserializeCamera3D(Camera3D& c, const json& json) {
  auto p = json["Position"];
  c.position = DeserializeVector3(p);
  auto t = json["Target"];
  c.target = DeserializeVector3(t);
  c.fovy = json["fovy"];
  c.projection = json["Projection"];
  auto u = json["up"];
  c.up = DeserializeVector3(u);
}
// Transform 2D
static json SerializeTransform2D(const TransformComponent2D& transform) {
  return {{"Position", SerializeVector2(transform.position)},
          {"Scale", SerializeVector2(transform.scale)},
          {"Rotation", transform.rotation}};
}
static void DeserializeTransform2D(TransformComponent2D& transform, const json& json) {
  auto p = json["Position"];
  transform.position = DeserializeVector2(p);
  auto s = json["Scale"];
  transform.scale = DeserializeVector2(s);
  transform.rotation = json["Rotation"];
}
// Transform3D
static json SerializeTransform3D(const TransformComponent3D& transform) {
  return {{"Position", SerializeVector3(transform.position)},
          {"Rotation", SerializeVector3(transform.rotation)},
          {"Scale", SerializeVector3(transform.scale)}};
}
static void DeserializeTransform3D(TransformComponent3D& transform, const json& json) {
  auto p = json["Position"];
  transform.position = DeserializeVector3(p);

  auto r = json["Rotation"];
  transform.rotation = DeserializeVector3(r);

  auto s = json["Scale"];
  transform.scale = DeserializeVector3(s);
}
// Tag
static json SerializeTag(const TagComponent& tag) { return {{"tag", tag.tag}}; }
static void DeserializeTag(TagComponent& tag, const json& json) { tag.tag = json["tag"]; }
// UUID
static json SerializeID(const IDComponent& id) { return {{"UUID", (uint64_t)id.ID}}; }
static void DeserializeID(IDComponent& id, const json& json) { id.ID = (uint64_t)json["UUID"]; }
// Name
static json SerializeName(const NameComponent& n) { return {{"Name", n.name}}; }
static void DeserializeName(NameComponent& n, const json& json) { n.name = json["Name"]; }
// Relationship
static json SerializeRelationship(const RelationshipComponent& r) {
  return {{"Parent", (uint64_t)r.parent}};
}
static void DeserializeRelationship(RelationshipComponent& r, const json& json) {
  r.parent = (uint64_t)json["Parent"];
}