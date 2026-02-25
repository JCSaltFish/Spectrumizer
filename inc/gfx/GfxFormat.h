/**
 * @file GfxFormat.h
 * @brief Header file for GfxFormat enumeration.
 */

#pragma once

/**
 * @brief Enumeration for graphics formats.
 */
enum class GfxFormat {
    UNDEFINED,
    R32_SFLOAT,
    R32G32_SFLOAT,
    R32G32B32_SFLOAT,
    R32G32B32A32_SFLOAT,
    R8_UNORM,
    R8G8B8A8_UNORM,
    R8_SNORM,
    R8G8B8A8_SNORM,
    D32_SFLOAT,
    D24_UNORM_S8_UINT,
    R32_UINT,
    R32G32_UINT,
    R32_SINT,
    R32G32_SINT,
};
