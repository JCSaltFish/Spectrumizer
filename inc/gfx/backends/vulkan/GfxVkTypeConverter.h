/**
 * @file GfxVkTypeConverter.h
 * @brief Header file for GfxVkTypeConverter class.
 * @details This class provides methods to convert Gfx types to Vulkan types.
 */

#pragma once

#include <vulkan/vulkan.h>

#include "gfx/GfxPub.h"

/**
 * @brief Converts Gfx types to Vulkan types.
 */
class GfxVkTypeConverter
{
public:
    /**
     * @brief Converts GfxFormat to VkFormat.
     * @param format The GfxFormat to convert.
     * @return The corresponding VkFormat.
     */
    static VkFormat toVkFormat(GfxFormat format);
    /**
     * @brief Gets the size in bytes of a given GfxFormat.
     * @param format The GfxFormat to get the size of.
     * @return The size in bytes of the format, or 0 if unsupported.
     */
    static int formatSize(GfxFormat format);
    /**
     * @brief Converts GfxPipelineState to VkDynamicState.
     * @param state The GfxPipelineState to convert.
     * @return The corresponding VkDynamicState.
     */
    static VkDynamicState toVkDynamicState(GfxPipelineState state);
};
