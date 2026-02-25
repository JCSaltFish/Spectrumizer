/**
 * @file GfxVkTypeConverter.cpp
 * @brief Implementation of GfxVkTypeConverter class.
 */

#include "gfx/backends/vulkan/GfxVkTypeConverter.h"

VkFormat GfxVkTypeConverter::toVkFormat(GfxFormat format) {
    switch (format) {
    case GfxFormat::R32_SFLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case GfxFormat::R32G32_SFLOAT:
        return VK_FORMAT_R32G32_SFLOAT;
    case GfxFormat::R32G32B32_SFLOAT:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case GfxFormat::R32G32B32A32_SFLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case GfxFormat::R8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case GfxFormat::R8G8B8A8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case GfxFormat::R8_SNORM:
        return VK_FORMAT_R8_SNORM;
    case GfxFormat::R8G8B8A8_SNORM:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case GfxFormat::D32_SFLOAT:
        return VK_FORMAT_D32_SFLOAT;
    case GfxFormat::D24_UNORM_S8_UINT:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case GfxFormat::R32_UINT:
        return VK_FORMAT_R32_UINT;
    case GfxFormat::R32G32_UINT:
        return VK_FORMAT_R32G32_UINT;
    case GfxFormat::R32_SINT:
        return VK_FORMAT_R32_SINT;
    case GfxFormat::R32G32_SINT:
        return VK_FORMAT_R32G32_SINT;
    case GfxFormat::UNDEFINED:
    default:
        return VK_FORMAT_UNDEFINED; // Unsupported format
    }
}

int GfxVkTypeConverter::formatSize(GfxFormat format) {
    switch (format) {
    case GfxFormat::R32_SFLOAT:
    case GfxFormat::R32_UINT:
    case GfxFormat::R32_SINT:
        return 4; // 32-bit per component
    case GfxFormat::R32G32_SFLOAT:
    case GfxFormat::R32G32_UINT:
    case GfxFormat::R32G32_SINT:
        return 8; // 2 components * 32-bit
    case GfxFormat::R32G32B32_SFLOAT:
        return 12; // 3 components * 32-bit
    case GfxFormat::R32G32B32A32_SFLOAT:
        return 16; // 4 components * 32-bit
    case GfxFormat::R8_UNORM:
    case GfxFormat::R8_SNORM:
        return 1; // 8-bit per component
    case GfxFormat::R8G8B8A8_UNORM:
    case GfxFormat::R8G8B8A8_SNORM:
        return 4; // 4 components * 8-bit
    case GfxFormat::D32_SFLOAT:
        return 4; // 32-bit depth
    case GfxFormat::D24_UNORM_S8_UINT:
        return 4; // 24-bit depth + 8-bit stencil packed in 32-bit
    case GfxFormat::UNDEFINED:
    default:
        return 0; // Unsupported or undefined format
    }
}

VkDynamicState GfxVkTypeConverter::toVkDynamicState(GfxPipelineState state) {
    switch (state) {
    case GfxPipelineState::VIEWPORT:
        return VK_DYNAMIC_STATE_VIEWPORT;
    case GfxPipelineState::SCISSOR:
        return VK_DYNAMIC_STATE_SCISSOR;
    case GfxPipelineState::LINE_WIDTH:
        return VK_DYNAMIC_STATE_LINE_WIDTH;
    case GfxPipelineState::DEPTH_BIAS:
        return VK_DYNAMIC_STATE_DEPTH_BIAS;
    case GfxPipelineState::BLEND_CONSTANTS:
        return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    case GfxPipelineState::STENCIL_COMPARE_MASK:
        return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
    case GfxPipelineState::STENCIL_WRITE_MASK:
        return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
    case GfxPipelineState::STENCIL_REFERENCE:
        return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
    case GfxPipelineState::CULL_MODE:
        return VK_DYNAMIC_STATE_CULL_MODE;
    case GfxPipelineState::FRONT_FACE:
        return VK_DYNAMIC_STATE_FRONT_FACE;
    case GfxPipelineState::PRIMITIVE_TOPOLOGY:
        return VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
    case GfxPipelineState::DEPTH_TEST_ENABLE:
        return VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE;
    case GfxPipelineState::DEPTH_WRITE_ENABLE:
        return VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;
    case GfxPipelineState::DEPTH_COMPARE_OP:
        return VK_DYNAMIC_STATE_DEPTH_COMPARE_OP;
    case GfxPipelineState::STENCIL_TEST_ENABLE:
        return VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE;
    case GfxPipelineState::STENCIL_OP:
        return VK_DYNAMIC_STATE_STENCIL_OP;
    case GfxPipelineState::DEPTH_BIAS_ENABLE:
        return VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
    case GfxPipelineState::PRIMITIVE_RESTART_ENABLE:
        return VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;
    case GfxPipelineState::LOGIC_OP:
        return VK_DYNAMIC_STATE_LOGIC_OP_EXT;
    case GfxPipelineState::POLYGON_MODE:
        return VK_DYNAMIC_STATE_POLYGON_MODE_EXT;
    case GfxPipelineState::LOGIC_OP_ENABLE:
        return VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT;
    case GfxPipelineState::COLOR_BLEND_ENABLE:
        return VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT;
    case GfxPipelineState::COLOR_BLEND_EQUATION:
        return VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;
    case GfxPipelineState::COLOR_WRITE_MASK:
        return VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT;
    case GfxPipelineState::LINE_SMOOTH_ENABLE:
        return VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT;
    default:
        return VK_DYNAMIC_STATE_MAX_ENUM; // Not a dynamic state
    }
};
