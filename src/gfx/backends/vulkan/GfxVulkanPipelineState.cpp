/**
 * @file GfxVulkanPipelineState.cpp
 * @brief Vulkan implementation of the GfxPipelineStateMachine interface.
 */

#include "gfx/backends/vulkan/GfxVulkanPipelineState.h"

void GfxVulkanPipelineStateMachine::setViewport(const GfxViewport& viewport) {
    m_stateCache.viewport = viewport;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::VIEWPORT)) {
        VkViewport vkViewport = {};
        vkViewport.x = static_cast<float>(viewport.x);
        vkViewport.y = static_cast<float>(viewport.y);
        vkViewport.width = static_cast<float>(viewport.width);
        vkViewport.height = static_cast<float>(viewport.height);
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;
        vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::VIEWPORT,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setScissor(const GfxRect& scissor) {
    m_stateCache.scissor = scissor;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::SCISSOR)) {
        VkRect2D vkScissor = {};
        vkScissor.offset.x = scissor.x;
        vkScissor.offset.y = scissor.y;
        vkScissor.extent.width = scissor.width;
        vkScissor.extent.height = scissor.height;
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::SCISSOR,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setLineWidth(float lineWidth) {
    m_stateCache.lineWidth = lineWidth;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::LINE_WIDTH)) {
        vkCmdSetLineWidth(m_commandBuffer, lineWidth);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::LINE_WIDTH,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setLineSmoothEnabled(bool enabled) {
    m_stateCache.lineSmoothEnabled = enabled;
    if (!m_currentBindingPipeline)
        return;
    if (!m_vkCmdSetLineRasterizationMode)
        return; // Ensure the function is loaded
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::LINE_SMOOTH_ENABLE)) {
        VkLineRasterizationModeEXT mode = enabled ?
            VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH :
            VK_LINE_RASTERIZATION_MODE_DEFAULT;
        m_vkCmdSetLineRasterizationMode(m_commandBuffer, mode);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::LINE_SMOOTH_ENABLE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setBlendConstants(const float blendConstants[4]) {
    memcpy(m_stateCache.blendConstants, blendConstants, sizeof(float) * 4);
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::BLEND_CONSTANTS)) {
        vkCmdSetBlendConstants(m_commandBuffer, blendConstants);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::BLEND_CONSTANTS,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setColorBlendEnabled(bool enabled) {
    m_stateCache.colorBlendEnabled = enabled;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::COLOR_BLEND_ENABLE)) {
        int attachmentCount =
            m_currentBindingPipeline->getRenderPass()->getColorAttachments().size();
        VkBool32 enabledVk = enabled ? VK_TRUE : VK_FALSE;
        std::vector<VkBool32> enables(attachmentCount, enabledVk);
        m_vkCmdSetColorBlendEnable(m_commandBuffer, 0, attachmentCount, enables.data());
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::COLOR_BLEND_ENABLE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setColorBlendEquation(const GfxBlendEquation& equation) {
    m_stateCache.colorBlendEquation = equation;
    if (!m_currentBindingPipeline)
        return;
    if (!m_vkCmdSetColorBlendEquation)
        return; // Ensure the function is loaded
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::COLOR_BLEND_EQUATION)) {
        int attachmentCount =
            m_currentBindingPipeline->getRenderPass()->getColorAttachments().size();
        VkColorBlendEquationEXT blendEquation = {};
        blendEquation.srcColorBlendFactor = static_cast<VkBlendFactor>(equation.srcColorFactor);
        blendEquation.dstColorBlendFactor = static_cast<VkBlendFactor>(equation.dstColorFactor);
        blendEquation.colorBlendOp = static_cast<VkBlendOp>(equation.colorBlendOp);
        blendEquation.srcAlphaBlendFactor = static_cast<VkBlendFactor>(equation.srcAlphaFactor);
        blendEquation.dstAlphaBlendFactor = static_cast<VkBlendFactor>(equation.dstAlphaFactor);
        blendEquation.alphaBlendOp = static_cast<VkBlendOp>(equation.alphaBlendOp);
        std::vector<VkColorBlendEquationEXT> blendEquations(attachmentCount, blendEquation);
        m_vkCmdSetColorBlendEquation(m_commandBuffer, 0, attachmentCount, blendEquations.data());
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::COLOR_BLEND_EQUATION,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setColorWriteMask(unsigned int mask) {
    m_stateCache.colorWriteMask = mask;
    if (!m_currentBindingPipeline)
        return;
    if (!m_vkCmdSetColorWriteMask)
        return; // Ensure the function is loaded
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::COLOR_WRITE_MASK)) {
        int attachmentCount =
            m_currentBindingPipeline->getRenderPass()->getColorAttachments().size();
        std::vector<VkColorComponentFlags> masks(attachmentCount, mask);
        m_vkCmdSetColorWriteMask(m_commandBuffer, 0, attachmentCount, masks.data());
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::COLOR_WRITE_MASK,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setDepthBiasEnabled(bool enabled) {
    m_stateCache.depthBiasEnabled = enabled;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::DEPTH_BIAS_ENABLE)) {
        vkCmdSetDepthBias(m_commandBuffer, 0.0f, 0.0f, 0.0f);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::DEPTH_BIAS_ENABLE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setDepthBiasParams(const GfxDepthBiasParams& params) {
    m_stateCache.depthBiasParams = params;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::DEPTH_BIAS)) {
        vkCmdSetDepthBias(
            m_commandBuffer,
            params.constantFactor,
            params.clamp,
            params.slopeFactor
        );
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::DEPTH_BIAS,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setDepthTestEnabled(bool enabled) {
    m_stateCache.depthTestEnabled = enabled;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::DEPTH_TEST_ENABLE)) {
        vkCmdSetDepthTestEnable(m_commandBuffer, enabled ? VK_TRUE : VK_FALSE);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::DEPTH_TEST_ENABLE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setDepthWriteEnabled(bool enabled) {
    m_stateCache.depthWriteEnabled = enabled;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::DEPTH_WRITE_ENABLE)) {
        vkCmdSetDepthWriteEnable(m_commandBuffer, enabled ? VK_TRUE : VK_FALSE);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::DEPTH_WRITE_ENABLE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setDepthCompareOp(GfxCompareOp op) {
    m_stateCache.depthCompareOp = op;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::DEPTH_COMPARE_OP)) {
        vkCmdSetDepthCompareOp(m_commandBuffer, static_cast<VkCompareOp>(op));
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::DEPTH_COMPARE_OP,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setStencilTestEnabled(bool enabled) {
    m_stateCache.stencilTestEnabled = enabled;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::STENCIL_TEST_ENABLE)) {
        vkCmdSetStencilTestEnable(m_commandBuffer, enabled ? VK_TRUE : VK_FALSE);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::STENCIL_TEST_ENABLE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setStencilOpParams
(
    GfxFaceSide face,
    const GfxStencilOpParams& params
) {
    if (face == GfxFaceSide::FRONT)
        m_stateCache.frontFaceStencilOpParams = params;
    else if (face == GfxFaceSide::BACK)
        m_stateCache.backFaceStencilOpParams = params;
    else
        return; // Invalid face side

    if (!m_currentBindingPipeline)
        return;

    VkStencilFaceFlagBits faceMask = static_cast<VkStencilFaceFlagBits>(face);
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::STENCIL_COMPARE_MASK)) {
        vkCmdSetStencilCompareMask(m_commandBuffer, faceMask, params.compareMask);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::STENCIL_COMPARE_MASK,
            m_stateCache
        );
    }
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::STENCIL_WRITE_MASK)) {
        vkCmdSetStencilWriteMask(m_commandBuffer, faceMask, params.writeMask);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::STENCIL_WRITE_MASK,
            m_stateCache
        );
    }
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::STENCIL_REFERENCE)) {
        vkCmdSetStencilReference(m_commandBuffer, faceMask, params.reference);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::STENCIL_REFERENCE,
            m_stateCache
        );
    }
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::STENCIL_OP)) {
        vkCmdSetStencilOp(
            m_commandBuffer,
            faceMask,
            static_cast<VkStencilOp>(params.failOp),
            static_cast<VkStencilOp>(params.passOp),
            static_cast<VkStencilOp>(params.depthFailOp),
            static_cast<VkCompareOp>(params.compareOp)
        );
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::STENCIL_OP,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setCullMode(GfxFaceSide mode) {
    m_stateCache.cullMode = mode;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::CULL_MODE)) {
        vkCmdSetCullMode(m_commandBuffer, static_cast<VkCullModeFlagBits>(mode));
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::CULL_MODE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setFrontFace(GfxFrontFace frontFace) {
    m_stateCache.frontFace = frontFace;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::FRONT_FACE)) {
        vkCmdSetFrontFace(m_commandBuffer, static_cast<VkFrontFace>(frontFace));
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::FRONT_FACE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setPrimitiveTopo(GfxPrimitiveTopo topo) {
    m_stateCache.primitiveTopo = topo;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::PRIMITIVE_TOPOLOGY)) {
        vkCmdSetPrimitiveTopology(m_commandBuffer, static_cast<VkPrimitiveTopology>(topo));
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::PRIMITIVE_TOPOLOGY,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setPrimitiveRestartEnabled(bool enabled) {
    m_stateCache.primitiveRestartEnabled = enabled;
    if (!m_currentBindingPipeline)
        return;
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::PRIMITIVE_RESTART_ENABLE)) {
        vkCmdSetPrimitiveRestartEnable(m_commandBuffer, enabled ? VK_TRUE : VK_FALSE);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::PRIMITIVE_RESTART_ENABLE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setLogicOpEnabled(bool enabled) {
    m_stateCache.logicOpEnabled = enabled;
    if (!m_currentBindingPipeline)
        return;
    if (!m_vkCmdSetLogicOpEnable)
        return; // Ensure the function is loaded
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::LOGIC_OP_ENABLE)) {
        m_vkCmdSetLogicOpEnable(m_commandBuffer, enabled ? VK_TRUE : VK_FALSE);
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::LOGIC_OP_ENABLE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setLogicOp(GfxLogicOp op) {
    m_stateCache.logicOp = op;
    if (!m_currentBindingPipeline)
        return;
    if (!m_vkCmdSetLogicOp)
        return; // Ensure the function is loaded
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::LOGIC_OP)) {
        m_vkCmdSetLogicOp(m_commandBuffer, static_cast<VkLogicOp>(op));
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::LOGIC_OP,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::setPolygonMode(GfxPolygonMode mode) {
    m_stateCache.polygonMode = mode;
    if (!m_currentBindingPipeline)
        return;
    if (!m_vkCmdSetPolygonMode)
        return; // Ensure the function is loaded
    if (m_currentBindingPipeline->hasDynamicState(GfxPipelineState::POLYGON_MODE)) {
        m_vkCmdSetPolygonMode(m_commandBuffer, static_cast<VkPolygonMode>(mode));
        GfxPipelineStateController::updatePipelineState(
            m_currentBindingPipeline,
            GfxPipelineState::POLYGON_MODE,
            m_stateCache
        );
    }
}

void GfxVulkanPipelineStateMachine::loadFunctions(VkDevice device) {
    m_vkCmdSetLogicOp = reinterpret_cast<PFN_vkCmdSetLogicOpEXT>
        (vkGetDeviceProcAddr(device, "vkCmdSetLogicOpEXT"));
    m_vkCmdSetPolygonMode = reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>
        (vkGetDeviceProcAddr(device, "vkCmdSetPolygonModeEXT"));
    m_vkCmdSetLogicOpEnable = reinterpret_cast<PFN_vkCmdSetLogicOpEnableEXT>
        (vkGetDeviceProcAddr(device, "vkCmdSetLogicOpEnableEXT"));
    m_vkCmdSetColorBlendEnable = reinterpret_cast<PFN_vkCmdSetColorBlendEnableEXT>
        (vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEnableEXT"));
    m_vkCmdSetColorBlendEquation = reinterpret_cast<PFN_vkCmdSetColorBlendEquationEXT>
        (vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEquationEXT"));
    m_vkCmdSetColorWriteMask = reinterpret_cast<PFN_vkCmdSetColorWriteMaskEXT>
        (vkGetDeviceProcAddr(device, "vkCmdSetColorWriteMaskEXT"));
    m_vkCmdSetLineRasterizationMode = reinterpret_cast<PFN_vkCmdSetLineRasterizationModeEXT>
        (vkGetDeviceProcAddr(device, "vkCmdSetLineRasterizationModeEXT"));
}
