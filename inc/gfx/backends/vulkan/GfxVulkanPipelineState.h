/**
 * @file GfxVulkanPipelineState.h
 * @brief Vulkan implementation of the GfxPipelineStateMachine interface.
 */

#pragma once

#include <vulkan/vulkan.h>

#include "gfx/GfxPr.h"

/**
 * @brief Vulkan implementation of the GfxPipelineStateMachine interface.
 */
class GfxVulkanPipelineStateMachine : public GfxPipelineStateMachine_T
{
public:
    void setViewport(const GfxViewport& viewport) override;
    void setScissor(const GfxRect& scissor) override;
    void setLineWidth(float lineWidth) override;
    void setLineSmoothEnabled(bool enabled) override;
    void setBlendConstants(const float blendConstants[4]) override;
    void setColorBlendEnabled(bool enabled) override;
    void setColorBlendEquation(const GfxBlendEquation& equation) override;
    void setColorWriteMask(unsigned int mask) override;
    void setDepthBiasEnabled(bool enabled) override;
    void setDepthBiasParams(const GfxDepthBiasParams& params) override;
    void setDepthTestEnabled(bool enabled) override;
    void setDepthWriteEnabled(bool enabled) override;
    void setDepthCompareOp(GfxCompareOp op) override;
    void setStencilTestEnabled(bool enabled) override;
    void setStencilOpParams(GfxFaceSide face, const GfxStencilOpParams& params) override;
    void setCullMode(GfxFaceSide mode) override;
    void setFrontFace(GfxFrontFace frontFace) override;
    void setPrimitiveTopo(GfxPrimitiveTopo topo) override;
    void setPrimitiveRestartEnabled(bool enabled) override;
    void setLogicOpEnabled(bool enabled) override;
    void setLogicOp(GfxLogicOp op) override;
    void setPolygonMode(GfxPolygonMode mode) override;

    void loadFunctions(VkDevice device);

public:
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE; // Vulkan command buffer for recording commands

    // Extended Vulkan functions for dynamic state setting
    PFN_vkCmdSetLogicOpEXT m_vkCmdSetLogicOp = nullptr;
    PFN_vkCmdSetPolygonModeEXT m_vkCmdSetPolygonMode = nullptr;
    PFN_vkCmdSetLogicOpEnableEXT m_vkCmdSetLogicOpEnable = nullptr;
    PFN_vkCmdSetColorBlendEnableEXT m_vkCmdSetColorBlendEnable = nullptr;
    PFN_vkCmdSetColorBlendEquationEXT m_vkCmdSetColorBlendEquation = nullptr;
    PFN_vkCmdSetColorWriteMaskEXT m_vkCmdSetColorWriteMask = nullptr;
    PFN_vkCmdSetLineRasterizationModeEXT m_vkCmdSetLineRasterizationMode = nullptr;
};
