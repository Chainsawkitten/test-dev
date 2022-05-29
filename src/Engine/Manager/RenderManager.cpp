#include "RenderManager.hpp"

#include <algorithm>
#include <Video/Renderer.hpp>
#include "Managers.hpp"
#include "ResourceManager.hpp"
#include "SoundManager.hpp"
#include "PhysicsManager.hpp"
#include "DebugDrawingManager.hpp"
#include "../Entity/Entity.hpp"
#include "../Component/Camera.hpp"
#include "../Component/Mesh.hpp"
#include "../Component/Material.hpp"
#include "../Component/DirectionalLight.hpp"
#include "../Component/PointLight.hpp"
#include "../Component/Shape.hpp"
#include "../Component/SpotLight.hpp"
#include "../Component/Sprite.hpp"
#include "../Component/SoundSource.hpp"
#include "../Geometry/Model.hpp"
#include "../Physics/Shape.hpp"
#include <Video/Geometry/Geometry3D.hpp>
#include "../Texture/TextureAsset.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <Video/RenderScene.hpp>
#include "../MainWindow.hpp"
#include "../Hymn.hpp"
#include <Utility/Profiling/Profiling.hpp>
#include "../Util/Json.hpp"
#include <Utility/Log.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Video/Texture/Texture2D.hpp>
#include <Video/LowLevelRenderer/Interface/Texture.hpp>
#include "Light.png.hpp"
#include "SoundSource.png.hpp"
#include "Camera.png.hpp"

using namespace Component;

RenderManager::RenderManager(Video::Renderer* renderer) {
    assert(renderer != nullptr);
    this->renderer = renderer;

    // Init textures.
    lightTexture = Managers().resourceManager->CreateTexture2D(LIGHT_PNG, LIGHT_PNG_LENGTH);
    soundSourceTexture = Managers().resourceManager->CreateTexture2D(SOUNDSOURCE_PNG, SOUNDSOURCE_PNG_LENGTH);
    cameraTexture = Managers().resourceManager->CreateTexture2D(CAMERA_PNG, CAMERA_PNG_LENGTH);
}

RenderManager::~RenderManager() {
    Managers().resourceManager->FreeTexture2D(lightTexture);
    Managers().resourceManager->FreeTexture2D(soundSourceTexture);
    Managers().resourceManager->FreeTexture2D(cameraTexture);
}

void RenderManager::Render(World& world, bool showSoundSources, bool showLightSources, bool showCameras, bool showPhysics, Entity* cameraEntity, bool lighting, bool showLightVolumes) {
    const glm::uvec2 windowSize = MainWindow::GetInstance()->GetSize();
    if (windowSize.x == 0 || windowSize.y == 0)
        return;

    renderer->BeginFrame();
    
    // Setup the render scene.
    Video::RenderScene renderScene;
    {
        PROFILE("Setup render scene");

        if (cameraEntity != nullptr) {
            // Camera specified.
            AddCamera(renderScene, *cameraEntity->GetComponent<Camera>(), windowSize);
        } else {
            // No camera specified. Render all the cameras.
            AddCameras(renderScene, windowSize);
        }

        if (!renderScene.cameras.empty()) {
            // Lights.
            AddLights(renderScene, lighting, showLightVolumes);

            // Meshes.
            AddMeshes(renderScene);

            // Sprites.
            AddSprites(renderScene);

            // Editor entities.
            if (showSoundSources || showLightSources || showCameras || showPhysics) {
                AddEditorEntities(renderScene, showSoundSources, showLightSources, showCameras, showPhysics);
            }

            // Debug shapes.
            AddDebugShapes(renderScene);
        }
    }

    renderer->Render(renderScene);
}

void RenderManager::UpdateBufferSize() {
    renderer->SetOutputSize(MainWindow::GetInstance()->GetSize());
}

Component::DirectionalLight* RenderManager::CreateDirectionalLight() {
    return directionalLights.Create();
}

Component::DirectionalLight* RenderManager::CreateDirectionalLight(const Json::Value& node) {
    Component::DirectionalLight* directionalLight = directionalLights.Create();

    // Load values from Json node.
    directionalLight->color = Json::LoadVec3(node["color"]);
    directionalLight->ambientCoefficient = node.get("ambientCoefficient", 0.5f).asFloat();

    return directionalLight;
}

const std::vector<Component::DirectionalLight*>& RenderManager::GetDirectionalLights() const {
    return directionalLights.GetAll();
}

Component::Camera* RenderManager::CreateCamera() {
    return cameras.Create();
}

Component::Camera* RenderManager::CreateCamera(const Json::Value& node) {
    Component::Camera* camera = cameras.Create();

    // Load values from Json node.
    camera->orthographic = node.get("orthographic", false).asBool();
    camera->fieldOfView = node.get("fieldOfView", 45.f).asFloat();
    camera->size = node.get("size", 10.0f).asFloat();
    camera->zNear = node.get("zNear", 0.1f).asFloat();
    camera->zFar = node.get("zFar", 100.f).asFloat();
    camera->viewport = Json::LoadVec4(node["viewport"]);
    camera->order = node.get("order", 0).asInt();
    camera->overlay = node.get("overlay", false).asBool();
    camera->layerMask = node.get("layerMask", 1u).asUInt();

    const Json::Value& filtersNode = node["filters"];
    camera->filterSettings.gamma = filtersNode.get("gamma", 2.2f).asFloat();
    camera->filterSettings.ditherApply = filtersNode["dither"].asBool();
    camera->filterSettings.fxaa = filtersNode["fxaa"].asBool();
    camera->filterSettings.bloom = filtersNode["bloom"].asBool();
    camera->filterSettings.bloomIntensity = filtersNode.get("bloomIntensity", 1.0f).asFloat();
    camera->filterSettings.bloomThreshold = filtersNode.get("bloomThreshold", 1.0f).asFloat();
    camera->filterSettings.bloomScatter = filtersNode.get("bloomScatter", 0.7f).asFloat();

    return camera;
}

const std::vector<Component::Camera*>& RenderManager::GetCameras() const {
    return cameras.GetAll();
}

Component::Material* RenderManager::CreateMaterial() {
    return materials.Create();
}

Component::Material* RenderManager::CreateMaterial(const Json::Value& node) {
    Component::Material* material = materials.Create();

    // Load values from Json node.
    LoadTexture(material->albedo, node.get("albedo", "").asString());
    LoadTexture(material->normal, node.get("normal", "").asString());
    LoadTexture(material->roughnessMetallic, node.get("roughnessMetallic", "").asString());

    return material;
}

const std::vector<Component::Material*>& RenderManager::GetMaterials() const {
    return materials.GetAll();
}

Component::Mesh* RenderManager::CreateMesh() {
    return meshes.Create();
}

Component::Mesh* RenderManager::CreateMesh(const Json::Value& node) {
    Component::Mesh* mesh = meshes.Create();

    // Load values from Json node.
    std::string meshName = node.get("model", "").asString();
    mesh->geometry = Managers().resourceManager->CreateModel(meshName);

    mesh->layerMask = node.get("layerMask", 1u).asUInt();

    return mesh;
}

const std::vector<Component::Mesh*>& RenderManager::GetMeshes() const {
    return meshes.GetAll();
}

Component::PointLight* RenderManager::CreatePointLight() {
    return pointLights.Create();
}

Component::PointLight* RenderManager::CreatePointLight(const Json::Value& node) {
    Component::PointLight* pointLight = pointLights.Create();

    // Load values from Json node.
    pointLight->color = Json::LoadVec3(node["color"]);
    pointLight->attenuation = node.get("attenuation", 1.f).asFloat();
    pointLight->intensity = node.get("intensity", 1.f).asFloat();
    pointLight->distance = node.get("distance", 1.f).asFloat();

    return pointLight;
}

const std::vector<Component::PointLight*>& RenderManager::GetPointLights() const {
    return pointLights.GetAll();
}

Component::SpotLight* RenderManager::CreateSpotLight() {
    return spotLights.Create();
}

Component::SpotLight* RenderManager::CreateSpotLight(const Json::Value& node) {
    Component::SpotLight* spotLight = spotLights.Create();

    // Load values from Json node.
    spotLight->color = Json::LoadVec3(node["color"]);
    spotLight->ambientCoefficient = node.get("ambientCoefficient", 0.5f).asFloat();
    spotLight->attenuation = node.get("attenuation", 1.f).asFloat();
    spotLight->intensity = node.get("intensity", 1.f).asFloat();
    spotLight->coneAngle = node.get("coneAngle", 15.f).asFloat();
    spotLight->distance = node.get("distance", 1.f).asFloat();

    return spotLight;
}

const std::vector<Component::SpotLight*>& RenderManager::GetSpotLights() const {
    return spotLights.GetAll();
}

Component::Sprite* RenderManager::CreateSprite() {
    return sprites.Create();
}

Component::Sprite* RenderManager::CreateSprite(const Json::Value& node) {
    Component::Sprite* sprite = sprites.Create();

    // Load values from Json node.
    LoadTexture(sprite->texture, node.get("texture", "").asString());
    sprite->pixelsPerUnit = node.get("pixelsPerUnit", 100.0f).asFloat();
    sprite->pivot = Json::LoadVec2(node["pivot"]);
    sprite->tint = Json::LoadVec3(node["tint"]);
    sprite->alpha = node.get("alpha", 1.0f).asFloat();
    sprite->layerMask = node.get("layerMask", 1u).asUInt();

    return sprite;
}

const std::vector<Component::Sprite*>& RenderManager::GetSprites() const {
    return sprites.GetAll();
}

void RenderManager::ClearKilledComponents() {
    directionalLights.ClearKilled();
    cameras.ClearKilled();
    materials.ClearKilled();
    meshes.ClearKilled();
    pointLights.ClearKilled();
    spotLights.ClearKilled();
    sprites.ClearKilled();
}

Video::Renderer* RenderManager::GetRenderer() {
    return renderer;
}

void RenderManager::AddCamera(Video::RenderScene& renderScene, const Component::Camera& camera, const glm::uvec2& windowSize) {
    Video::RenderScene::Camera renderCamera = {};

    Entity* cameraEntity = camera.entity;
    renderCamera.position = cameraEntity->GetWorldPosition();
    renderCamera.viewMatrix = glm::inverse(cameraEntity->GetModelMatrix());
    renderCamera.projectionMatrix = camera.GetProjection(glm::vec2(windowSize) * glm::vec2(camera.viewport.z, camera.viewport.w));
    renderCamera.viewProjectionMatrix = renderCamera.projectionMatrix * renderCamera.viewMatrix;
    renderCamera.zNear = camera.zNear;
    renderCamera.zFar = camera.zFar;
    renderCamera.viewport = camera.viewport;
    renderCamera.overlay = camera.overlay;
    renderCamera.layerMask = camera.layerMask;

    renderCamera.postProcessingConfiguration.gamma = camera.filterSettings.gamma;
    renderCamera.postProcessingConfiguration.fxaa.enabled = camera.filterSettings.fxaa;
    renderCamera.postProcessingConfiguration.dither.enabled = camera.filterSettings.ditherApply;
    renderCamera.postProcessingConfiguration.bloom.enabled = camera.filterSettings.bloom;
    renderCamera.postProcessingConfiguration.bloom.intensity = camera.filterSettings.bloomIntensity;
    renderCamera.postProcessingConfiguration.bloom.threshold = camera.filterSettings.bloomThreshold;
    renderCamera.postProcessingConfiguration.bloom.scatter = camera.filterSettings.bloomScatter;

    renderScene.cameras.push_back(renderCamera);
}

void RenderManager::AddCameras(Video::RenderScene& renderScene, const glm::uvec2& windowSize) {
    // Sort cameras according to the order.
    std::vector<Camera*> sortedCameras = cameras.GetAll();
    std::sort(sortedCameras.begin(), sortedCameras.end(),
        [](const Camera* a, const Camera* b) -> bool {
            return a->order < b->order;
        });

    // Add cameras to render scene.
    for (Camera* camera : sortedCameras) {
        if (camera->entity->IsEnabled()) {
            AddCamera(renderScene, *camera, windowSize);
        }
    }
}

void RenderManager::AddLights(Video::RenderScene& renderScene, bool lighting, bool showLightVolumes) {
    if (lighting) {
        // Cull lights and update light list.
        AddWorldLights(renderScene, showLightVolumes);
    } else {
        // Use full ambient light and ignore lights in the scene.
        AddAmbientLight(renderScene);
    }
}

void RenderManager::AddWorldLights(Video::RenderScene& renderScene, bool showLightVolumes) {
    // Add all directional lights.
    for (Component::DirectionalLight* directionalLight : directionalLights.GetAll()) {
        if (directionalLight->IsKilled() || !directionalLight->entity->IsEnabled())
            continue;

        Video::RenderScene::DirectionalLight light;

        Entity* lightEntity = directionalLight->entity;
        light.direction = lightEntity->GetDirection();
        light.color = directionalLight->color;
        light.ambientCoefficient = directionalLight->ambientCoefficient;

        renderScene.directionalLights.push_back(light);
    }

    // Add all spot lights.
    for (Component::SpotLight* spotLight : spotLights.GetAll()) {
        if (spotLight->IsKilled() || !spotLight->entity->IsEnabled())
            continue;

        Video::RenderScene::SpotLight light;

        Entity* lightEntity = spotLight->entity;
        light.position = lightEntity->GetWorldPosition();
        light.distance = spotLight->distance;
        light.color = spotLight->color;
        light.intensity = spotLight->intensity;
        light.attenuation = spotLight->attenuation;
        light.direction = lightEntity->GetDirection();
        light.coneAngle = spotLight->coneAngle;

        renderScene.spotLights.push_back(light);

        if (showLightVolumes)
            Managers().debugDrawingManager->AddSphere(light.position, spotLight->distance, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    // Add all point lights.
    for (Component::PointLight* pointLight : pointLights.GetAll()) {
        if (pointLight->IsKilled() || !pointLight->entity->IsEnabled())
            continue;

        Video::RenderScene::PointLight light;

        Entity* lightEntity = pointLight->entity;
        light.position = lightEntity->GetWorldPosition();
        light.distance = pointLight->distance;
        light.color = pointLight->color;
        light.intensity = pointLight->intensity;
        light.attenuation = pointLight->attenuation;

        renderScene.pointLights.push_back(light);

        if (showLightVolumes)
            Managers().debugDrawingManager->AddSphere(light.position, pointLight->distance, glm::vec3(1.0f, 1.0f, 1.0f));
    }
}

void RenderManager::AddAmbientLight(Video::RenderScene& renderScene) {
    Video::RenderScene::DirectionalLight light;
    light.direction = glm::vec3(1.0f, 0.0f, 0.0f);
    light.color = glm::vec3(0.0f, 0.0f, 0.0f);
    light.ambientCoefficient = 1.0f;

    renderScene.directionalLights.push_back(light);
}

void RenderManager::AddMeshes(Video::RenderScene& renderScene) {
    const std::vector<Mesh*>& meshComponents = meshes.GetAll();

    for (Mesh* meshComp : meshComponents) {
        Entity* entity = meshComp->entity;
        if (entity->IsKilled() || !entity->IsEnabled())
            continue;

        if (meshComp->geometry == nullptr || meshComp->geometry->GetIndexCount() == 0)
            continue;

        Material* material = entity->GetComponent<Material>();
        if (material == nullptr)
            continue;

        Video::RenderScene::Mesh mesh;
        mesh.modelMatrix = entity->GetModelMatrix();
        mesh.geometry = meshComp->geometry;
        mesh.axisAlignedBoundingBox = meshComp->geometry->GetAxisAlignedBoundingBox();
        mesh.albedo = material->albedo->GetTexture();
        mesh.normal = material->normal->GetTexture();
        mesh.roughnessMetallic = material->roughnessMetallic->GetTexture();
        mesh.layerMask = meshComp->layerMask;

        renderScene.meshes.push_back(mesh);
    }
}

void RenderManager::AddEditorEntities(Video::RenderScene& renderScene, bool showSoundSources, bool showLightSources, bool showCameras, bool showPhysics) {
    // Sound sources.
    if (showSoundSources) {
        Video::RenderScene::Icon icon;
        icon.texture = soundSourceTexture;

        for (SoundSource* soundSource : Managers().soundManager->GetSoundSources())
            icon.positions.push_back(soundSource->entity->GetWorldPosition());

        renderScene.icons.push_back(icon);
    }

    // Light sources.
    if (showLightSources) {
        Video::RenderScene::Icon icon;
        icon.texture = lightTexture;

        for (DirectionalLight* light : directionalLights.GetAll())
            icon.positions.push_back(light->entity->GetWorldPosition());

        for (PointLight* light : pointLights.GetAll())
            icon.positions.push_back(light->entity->GetWorldPosition());

        for (SpotLight* light : spotLights.GetAll())
            icon.positions.push_back(light->entity->GetWorldPosition());

        renderScene.icons.push_back(icon);
    }

    // Cameras.
    if (showCameras) {
        Video::RenderScene::Icon icon;
        icon.texture = cameraTexture;

        for (Camera* camera : cameras.GetAll())
            icon.positions.push_back(camera->entity->GetWorldPosition());

        renderScene.icons.push_back(icon);
    }

    // Physics.
    if (showPhysics) {
        for (Component::Shape* shapeComp : Managers().physicsManager->GetShapeComponents()) {
            const ::Physics::Shape& shape = *shapeComp->GetShape();
            if (shape.GetKind() == ::Physics::Shape::Kind::Sphere) {
                Managers().debugDrawingManager->AddSphere(shapeComp->entity->GetWorldPosition(), shape.GetSphereData()->radius, glm::vec3(1.0f, 1.0f, 1.0f));
            } else if (shape.GetKind() == ::Physics::Shape::Kind::Plane) {
                glm::vec3 normal = shapeComp->entity->GetModelMatrix() * glm::vec4(shape.GetPlaneData()->normal, 0.0f);
                Managers().debugDrawingManager->AddPlane(shapeComp->entity->GetWorldPosition(), normal, glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
            } else if (shape.GetKind() == ::Physics::Shape::Kind::Box) {
                glm::vec3 dimensions(shape.GetBoxData()->width, shape.GetBoxData()->height, shape.GetBoxData()->depth);
                glm::vec3 position = shapeComp->entity->GetWorldPosition();
                glm::quat orientation = shapeComp->entity->GetWorldOrientation();
                glm::mat4 transformationMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation);
                Managers().debugDrawingManager->AddCuboid(dimensions, transformationMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
            } else if (shape.GetKind() == ::Physics::Shape::Kind::Cylinder) {
                glm::vec3 position = shapeComp->entity->GetWorldPosition();
                glm::quat orientation = shapeComp->entity->GetWorldOrientation();
                glm::mat4 transformationMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation);
                Managers().debugDrawingManager->AddCylinder(shape.GetCylinderData()->radius, shape.GetCylinderData()->length, transformationMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
            } else if (shape.GetKind() == ::Physics::Shape::Kind::Cone) {
                glm::vec3 position = shapeComp->entity->GetWorldPosition();
                glm::quat orientation = shapeComp->entity->GetWorldOrientation();
                glm::mat4 transformationMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation);
                Managers().debugDrawingManager->AddCone(shape.GetConeData()->radius, shape.GetConeData()->height, transformationMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
            }
        }
    }
}

void RenderManager::AddDebugShapes(Video::RenderScene& renderScene) {
    renderScene.debugDrawingPoints = Managers().debugDrawingManager->GetPoints();
    renderScene.debugDrawingLines = Managers().debugDrawingManager->GetLines();
    renderScene.debugDrawingCuboids = Managers().debugDrawingManager->GetCuboids();
    renderScene.debugDrawingPlanes = Managers().debugDrawingManager->GetPlanes();
    renderScene.debugDrawingCircles = Managers().debugDrawingManager->GetCircles();
    renderScene.debugDrawingSpheres = Managers().debugDrawingManager->GetSpheres();
    renderScene.debugDrawingCylinders = Managers().debugDrawingManager->GetCylinders();
    renderScene.debugDrawingCones = Managers().debugDrawingManager->GetCones();
    renderScene.debugDrawingMeshes = Managers().debugDrawingManager->GetMeshes();
}

void RenderManager::AddSprites(Video::RenderScene& renderScene) {
    const std::vector<Sprite*>& spriteComponents = sprites.GetAll();

    for (Sprite* spriteComp : spriteComponents) {
        Entity* entity = spriteComp->entity;
        if (entity->IsKilled() || !entity->IsEnabled())
            continue;

        Video::RenderScene::Sprite sprite;
        sprite.texture = spriteComp->texture->GetTexture();
        sprite.modelMatrix = entity->GetModelMatrix();
        sprite.size = glm::vec2(sprite.texture->GetTexture()->GetSize()) / spriteComp->pixelsPerUnit;
        sprite.pivot = spriteComp->pivot;
        sprite.tint = glm::vec4(spriteComp->tint, spriteComp->alpha);
        sprite.layerMask = spriteComp->layerMask;

        renderScene.sprites.push_back(sprite);
    }
}

void RenderManager::LoadTexture(TextureAsset*& texture, const std::string& name) {
    if (!name.empty())
        texture = Managers().resourceManager->CreateTextureAsset(name);
}
