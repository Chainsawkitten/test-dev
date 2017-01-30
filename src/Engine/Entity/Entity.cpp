#include "Entity.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include "../Component/Animation.hpp"
#include "../Component/Transform.hpp"
#include "../Component/Lens.hpp"
#include "../Component/Mesh.hpp"
#include "../Component/Material.hpp"
#include "../Component/DirectionalLight.hpp"
#include "../Component/PointLight.hpp"
#include "../Component/SpotLight.hpp"
#include "../Component/Physics.hpp"
#include "../Component/Listener.hpp"
#include "../Component/Script.hpp"
#include "../Component/SoundSource.hpp"
#include "../Component/ParticleEmitter.hpp"
#include "../Util/Json.hpp"

Entity::Entity(Scene* scene, const std::string& name) {
    this->scene = scene;
    this->name = name;
}

Entity::~Entity() {
    
}

void Entity::Kill() {
    killed = true;
    
    for (auto& it : components)
        it.second->Kill();
}

bool Entity::IsKilled() const {
    return killed;
}

Json::Value Entity::Save() const {
    Json::Value entity;
    entity["name"] = name;
    entity["position"] = Json::SaveVec3(position);
    entity["scale"] = Json::SaveVec3(scale);
    entity["rotation"] = Json::SaveVec3(rotation);
    
    Save<Component::Animation>(entity, "Animation");
    Save<Component::Transform>(entity, "Transform");
    Save<Component::Lens>(entity, "Lens");
    Save<Component::Mesh>(entity, "Mesh");
    Save<Component::Material>(entity, "Material");
    Save<Component::DirectionalLight>(entity, "DirectionalLight");
    Save<Component::PointLight>(entity, "PointLight");
    Save<Component::SpotLight>(entity, "SpotLight");
    Save<Component::Physics>(entity, "Physics");
    Save<Component::Listener>(entity, "Listener");
    Save<Component::Script>(entity, "Script");
    Save<Component::SoundSource>(entity, "SoundSource");
    Save<Component::ParticleEmitter>(entity, "ParticleEmitter");
    
    return entity;
}

void Entity::Load(const Json::Value& node) {
    name = node.get("name", "").asString();
    position = Json::LoadVec3(node["position"]);
    scale = Json::LoadVec3(node["scale"]);
    rotation = Json::LoadVec3(node["rotation"]);
    
    Load<Component::Animation>(node, "Animation");
    Load<Component::Transform>(node, "Transform");
    Load<Component::Lens>(node, "Lens");
    Load<Component::Mesh>(node, "Mesh");
    Load<Component::Material>(node, "Material");
    Load<Component::DirectionalLight>(node, "DirectionalLight");
    Load<Component::PointLight>(node, "PointLight");
    Load<Component::SpotLight>(node, "SpotLight");
    Load<Component::Physics>(node, "Physics");
    Load<Component::Listener>(node, "Listener");
    Load<Component::Script>(node, "Script");
    Load<Component::SoundSource>(node, "SoundSource");
    Load<Component::ParticleEmitter>(node, "ParticleEmitter");
}

glm::mat4 Entity::GetModelMatrix() const {
    glm::mat4 orientation;
    orientation = glm::rotate(orientation, glm::radians(rotation.x), glm::vec3(0.f, 1.f, 0.f));
    orientation = glm::rotate(orientation, glm::radians(rotation.y), glm::vec3(1.f, 0.f, 0.f));
    orientation = glm::rotate(orientation, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
    return glm::translate(glm::mat4(), position) * orientation * glm::scale(glm::mat4(), scale);
}

glm::mat4 Entity::GetOrientation() const {
    glm::mat4 orientation;
    orientation = glm::rotate(orientation, glm::radians(rotation.x), glm::vec3(0.f, 1.f, 0.f));
    orientation = glm::rotate(orientation, glm::radians(rotation.y), glm::vec3(1.f, 0.f, 0.f));
    return glm::rotate(orientation, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
}

glm::mat4 Entity::GetCameraOrientation() const {
    glm::mat4 orientation;
    orientation = glm::rotate(orientation, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
    orientation = glm::rotate(orientation, glm::radians(rotation.y), glm::vec3(1.f, 0.f, 0.f));
    return glm::rotate(orientation, glm::radians(rotation.x), glm::vec3(0.f, 1.f, 0.f));
}

glm::vec3 Entity::GetDirection() const {
    return glm::normalize(glm::vec3(GetOrientation() * glm::vec4(0.f, 0.f, 1.f, 0.f)));
}
