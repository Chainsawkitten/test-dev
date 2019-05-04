#pragma once

#include "../Interface/LowLevelRenderer.hpp"

struct GLFWwindow;

namespace Video {

class OpenGLShaderProgram;

/// Low-level renderer implementing OpenGL.
class OpenGLRenderer : public LowLevelRenderer {
  public:
    /// Create new OpenGL renderer.
    /**
     * @param window The window to render to.
     */
    explicit OpenGLRenderer(GLFWwindow* window);

    /// Destructor.
    ~OpenGLRenderer() final;

    CommandBuffer* CreateCommandBuffer() final;
    void BeginFrame() final;
    void Submit(CommandBuffer* commandBuffer) final;
    void Present() final;
    Buffer* CreateBuffer(Buffer::BufferUsage bufferUsage, unsigned int size, const void* data = nullptr) final;
    VertexDescription* CreateVertexDescription(unsigned int attributeCount, const VertexDescription::Attribute* attributes, bool indexBuffer = false) final;
    GeometryBinding* CreateGeometryBinding(const VertexDescription* vertexDescription, const Buffer* vertexBuffer, GeometryBinding::IndexType indexType = GeometryBinding::IndexType::NONE, const Buffer* indexBuffer = nullptr) final;
    Shader* CreateShader(const ShaderSource& shaderSource, Shader::Type type) final;
    ShaderProgram* CreateShaderProgram(std::initializer_list<const Shader*> shaders) final;
    Texture* CreateTexture(const glm::uvec2 size, Texture::Type type, Texture::Format format, int components = 0, unsigned char* data = nullptr) final;
    RenderPass* CreateRenderPass(Texture* colorAttachment, RenderPass::LoadOperation colorLoadOperation, Texture* depthAttachment, RenderPass::LoadOperation depthLoadOperation) final;
    GraphicsPipeline* CreateGraphicsPipeline(const ShaderProgram* shaderProgram, const GraphicsPipeline::Configuration& configuration, const VertexDescription* vertexDescription = nullptr) final;
    void Wait() final;
    char* ReadImage(RenderPass* renderPass) final;

    /// Get the shader program used for blitting.
    /**
     * @return The blit shader program.
     */
    const OpenGLShaderProgram* GetBlitShaderProgram() const;

  private:
    OpenGLRenderer(const OpenGLRenderer& other) = delete;

    GLFWwindow* window;

    // Blitting to swap chain.
    Shader* blitVertexShader;
    Shader* blitFragmentShader;
    ShaderProgram* blitShaderProgram;
};

} // namespace Video
