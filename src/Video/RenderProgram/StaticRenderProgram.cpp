#include "StaticRenderProgram.hpp"

#include "../Geometry/Geometry3D.hpp"
#include "../Texture/Texture2D.hpp"
#include "../Culling/Frustum.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "../Shader/Shader.hpp"
#include "../Shader/ShaderProgram.hpp"
#include "Default3D.vert.hpp"
#include "Default3D.frag.hpp"
#include "ZrejectionStatic.vert.hpp"
#include "Zrejection.frag.hpp"
#include "ShadowStatic.vert.hpp"
#include "../Buffer/StorageBuffer.hpp"
#include <chrono>

using namespace Video;

StaticRenderProgram::StaticRenderProgram() {
    Shader* vertexShader = new Shader(DEFAULT3D_VERT, DEFAULT3D_VERT_LENGTH, GL_VERTEX_SHADER);
    Shader* fragmentShader = new Shader(DEFAULT3D_FRAG, DEFAULT3D_FRAG_LENGTH, GL_FRAGMENT_SHADER);
    shaderProgram = new ShaderProgram({ vertexShader, fragmentShader });
    delete vertexShader;
    delete fragmentShader;

    // Create shaders for early rejection pass
    vertexShader = new Shader(ZREJECTIONSTATIC_VERT, ZREJECTIONSTATIC_VERT_LENGTH, GL_VERTEX_SHADER);
    fragmentShader = new Shader(ZREJECTION_FRAG, ZREJECTION_FRAG_LENGTH, GL_FRAGMENT_SHADER);
    zShaderProgram = new ShaderProgram({ vertexShader, fragmentShader });
    delete vertexShader;

    // Create shaders for shadowpass
    vertexShader = new Shader(SHADOWSTATIC_VERT, SHADOWSTATIC_VERT_LENGTH, GL_VERTEX_SHADER);
    shadowProgram = new ShaderProgram({ vertexShader, fragmentShader });
    delete vertexShader;
    delete fragmentShader;
    
    // Get uniform locations.
    shadowLightSpaceLocation = shadowProgram->GetUniformLocation("lightSpaceMatrix");
    shadowModelLocation = shadowProgram->GetUniformLocation("model");
    zViewProjectionLocation = zShaderProgram->GetUniformLocation("viewProjection");
    zModelLocation = zShaderProgram->GetUniformLocation("model");
    viewProjectionLocation = shaderProgram->GetUniformLocation("viewProjection");
    lightSpaceLocation = shaderProgram->GetUniformLocation("lightSpaceMatrix");
    lightCountLocation = shaderProgram->GetUniformLocation("lightCount");
    cameraNearPlaneLocation = shaderProgram->GetUniformLocation("cameraNear");
    cameraFarPlaneLocation = shaderProgram->GetUniformLocation("cameraFar");
    gammaLocation = shaderProgram->GetUniformLocation("gamma");
    fogApplyLocation = shaderProgram->GetUniformLocation("fogApply");
    fogDensityLocation = shaderProgram->GetUniformLocation("fogDensity");
    fogColorLocation = shaderProgram->GetUniformLocation("fogColor");
    colorFilterApplyLocation = shaderProgram->GetUniformLocation("colorFilterApply");
    colorFilterColorLocation = shaderProgram->GetUniformLocation("colorFilterColor");
    ditherApplyLocation = shaderProgram->GetUniformLocation("ditherApply");
    timeLocation = shaderProgram->GetUniformLocation("time");
    frameSizeLocation = shaderProgram->GetUniformLocation("frameSize");
    mapAlbedoLocation = shaderProgram->GetUniformLocation("mapAlbedo");
    mapNormalLocation = shaderProgram->GetUniformLocation("mapNormal");
    mapMetallicLocation = shaderProgram->GetUniformLocation("mapMetallic");
    mapRoughnessLocation = shaderProgram->GetUniformLocation("mapRoughness");
    mapShadowLocation = shaderProgram->GetUniformLocation("mapShadow");
    modelLocation = shaderProgram->GetUniformLocation("model");
    viewLocation = shaderProgram->GetUniformLocation("viewMatrix");
    normalLocation = shaderProgram->GetUniformLocation("normalMatrix");
}

StaticRenderProgram::~StaticRenderProgram() {
    delete shaderProgram;
    delete zShaderProgram;
    delete shadowProgram;
}

void StaticRenderProgram::PreShadowRender(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, int shadowId, unsigned int shadowMapSize, int depthFbo) {
    // Cull front faces to avoid peter panning.
    glCullFace(GL_FRONT);
    glViewport(0, 0, shadowMapSize, shadowMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    this->shadowProgram->Use();

    this->viewMatrix = viewMatrix;
    this->projectionMatrix = projectionMatrix;
    this->lightSpaceMatrix = projectionMatrix * viewMatrix;
    this->viewProjectionMatrix = projectionMatrix * viewMatrix;
    this->shadowId = shadowId;

    glUniformMatrix4fv(shadowLightSpaceLocation, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
}

void StaticRenderProgram::ShadowRender(Geometry::Geometry3D* geometry, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::mat4& modelMatrix) const {
    Frustum frustum(viewProjectionMatrix * modelMatrix);
    if (frustum.Collide(geometry->GetAxisAlignedBoundingBox())) {
        glBindVertexArray(geometry->GetVertexArray());

        glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);

        glDrawElements(GL_TRIANGLES, geometry->GetIndexCount(), GL_UNSIGNED_INT, (void*)0);
    }
}


void StaticRenderProgram::PreDepthRender(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    this->zShaderProgram->Use();

    this->viewMatrix = viewMatrix;
    this->projectionMatrix = projectionMatrix;
    this->viewProjectionMatrix = projectionMatrix * viewMatrix;

    glUniformMatrix4fv(zViewProjectionLocation, 1, GL_FALSE, &viewProjectionMatrix[0][0]);
}

void StaticRenderProgram::DepthRender(Geometry::Geometry3D* geometry, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::mat4& modelMatrix) const {
    Frustum frustum(viewProjectionMatrix * modelMatrix);
    if (frustum.Collide(geometry->GetAxisAlignedBoundingBox())) {
        glBindVertexArray(geometry->GetVertexArray());

        glUniformMatrix4fv(zModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);

        glDrawElements(GL_TRIANGLES, geometry->GetIndexCount(), GL_UNSIGNED_INT, (void*)0);
    }
}

void StaticRenderProgram::PreRender(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const StorageBuffer* lightBuffer, unsigned int lightCount, float cameraNear, float cameraFar) {
    this->shaderProgram->Use();
    this->viewMatrix = viewMatrix;
    this->projectionMatrix = projectionMatrix;
    this->viewProjectionMatrix = projectionMatrix * viewMatrix;

    // Matrices.
    glUniformMatrix4fv(viewProjectionLocation, 1, GL_FALSE, &viewProjectionMatrix[0][0]);
    glUniformMatrix4fv(lightSpaceLocation, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    // Lights.
    glUniform1i(lightCountLocation, lightCount);
    lightBuffer->BindBase(5);

    // Image processing.
    glUniform1fv(cameraNearPlaneLocation, 1, &cameraNear);
    glUniform1fv(cameraFarPlaneLocation, 1, &cameraFar);
    glUniform1fv(gammaLocation, 1, &gamma);

    glUniform1iv(fogApplyLocation, 1, &fogApply);
    glUniform1fv(fogDensityLocation, 1, &fogDensity);
    glUniform3fv(fogColorLocation, 1, &fogColor[0]);

    glUniform1iv(colorFilterApplyLocation, 1, &colorFilterApply);
    glUniform3fv(colorFilterColorLocation, 1, &colorFilterColor[0]);

    float time = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() % 30000000000 / 1000000000.0);
    glUniform1iv(ditherApplyLocation, 1, &ditherApply);

    glUniform1fv(timeLocation, 1, &time);
    glUniform2fv(frameSizeLocation, 1, &frameSize[0]);
}

void StaticRenderProgram::Render(Geometry::Geometry3D* geometry, const Video::Texture2D* textureAlbedo, const Video::Texture2D* normalTexture, const Video::Texture2D* textureMetallic, const Video::Texture2D* textureRoughness, const glm::mat4& modelMatrix) const {
    Frustum frustum(viewProjectionMatrix * modelMatrix);
    if (frustum.Collide(geometry->GetAxisAlignedBoundingBox())) {
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        glBindVertexArray(geometry->GetVertexArray());

        // Set texture locations
        glUniform1i(mapAlbedoLocation, 0);
        glUniform1i(mapNormalLocation, 1);
        glUniform1i(mapMetallicLocation, 2);
        glUniform1i(mapRoughnessLocation, 3);
        glUniform1i(mapShadowLocation, 4);

        // Textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureAlbedo->GetTextureID());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTexture->GetTextureID());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureMetallic->GetTextureID());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureRoughness->GetTextureID());
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, shadowId);

        // Render model.
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &viewMatrix[0][0]);
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(viewMatrix * modelMatrix));
        glUniformMatrix3fv(normalLocation, 1, GL_FALSE, &glm::mat3(normalMatrix)[0][0]);
        

        glDrawElements(GL_TRIANGLES, geometry->GetIndexCount(), GL_UNSIGNED_INT, (void*)0);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
}
