#pragma once

#include "../Interface/CommandBuffer.hpp"
#include <vulkan/vulkan.h>
#include <map>
#include <vector>

namespace Video {

class VulkanRenderer;
class VulkanRenderPass;
class VulkanGraphicsPipeline;
class VulkanTexture;

/// Vulkan implementation of CommandBuffer.
class VulkanCommandBuffer : public CommandBuffer {
  public:
    /// Create new Vulkan command buffer.
    /**
     * @param vulkanRenderer The Vulkan renderer.
     * @param device The Vulkan device.
     * @param commandPool The command pool to allocate the command buffer from.
     */
    VulkanCommandBuffer(VulkanRenderer* vulkanRenderer, VkDevice device, VkCommandPool commandPool);

    /// Destructor.
    ~VulkanCommandBuffer() final;

    void BeginRenderPass(RenderPass* renderPass) final;
    void EndRenderPass() final;
    void BindGraphicsPipeline(GraphicsPipeline* graphicsPipeline) final;
    void SetViewport(const glm::uvec2& origin, const glm::uvec2& size) final;
    void SetScissor(const glm::uvec2& origin, const glm::uvec2& size) final;
    void SetLineWidth(float width) final;
    void BindGeometry(const GeometryBinding* geometryBinding) final;
    void BindUniformBuffer(ShaderProgram::BindingType bindingType, Buffer* uniformBuffer) final;
    void BindStorageBuffer(Buffer* storageBuffer) final;
    void BindMaterial(std::initializer_list<const Texture*> textures) final;
    void PushConstants(const void* data) final;
    void Draw(unsigned int vertexCount, unsigned int firstVertex) final;
    void DrawIndexed(unsigned int indexCount, unsigned int firstIndex, unsigned int baseVertex) final;
    void BlitToSwapChain(Texture* texture) final;

    /// Get command buffer.
    /**
     * @return The internal Vulkan command buffer.
     */
    VkCommandBuffer GetCommandBuffer() const;

    /// Finish recording the command buffer.
    void End();

    /// Cycle the command buffer for the next frame.
    void NextFrame();

    /// Get whether the command buffer contains a blit to the swap chain.
    /**
     * @return Whether the command buffer contains a blit to the swap chain.
     */
    bool ContainsBlitToSwapChain() const;

  private:
    VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;

    void Begin();

    void TransitionTexture(const VulkanTexture* texture, VkImageLayout destinationImageLayout);
    void ResetImageLayouts();

    VulkanRenderer* vulkanRenderer;
    VkDevice device;
    VkCommandPool commandPool;

    VkCommandBuffer* commandBuffer;
    VkCommandBuffer renderPassCommandBuffer;
    std::vector<VkCommandBuffer>* secondaryCommandBuffers;
    uint32_t currentFrame = 0;
    uint32_t swapChainImages;

    bool ended = false;
    bool inRenderPass = false;
    bool containsBlitToSwapChain = false;

    VkClearValue clearValues[2];
    VkRenderPassBeginInfo renderPassBeginInfo;

    const VulkanRenderPass* currentRenderPass = nullptr;
    VulkanGraphicsPipeline* currentGraphicsPipeline = nullptr;

    std::map<const VulkanTexture*, VkImageLayout> renderTextureStates;
};

} // namespace Video
