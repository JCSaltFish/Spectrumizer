/**
 * @file Image.h
 * @brief Header file for the ImageRGBA utility.
 */

#pragma once

#include "UtilsCommon.h"

namespace ImageRGBA {

/**
 * @brief Supported image formats for loading and saving.
 */
enum class Format {
    BMP,
    JPG,
    PNG,
    TGA,
};

/**
 * @brief Load an RGBA image from a file.
 * @param filename The path to the image file.
 * @param[out] width Output parameter for the image width.
 * @param[out] height Output parameter for the image height.
 * @param[out] pixels Output vector to hold the pixel data (RGBA format).
 * @return 0 on success, non-zero on failure.
 */
int loadFromFile(
    const std::string& filename,
    int& width,
    int& height,
    std::vector<unsigned char>& pixels
);
/**
 * @brief Load an RGBA image from a memory buffer.
 * @param buffer Pointer to the memory buffer containing image data.
 * @param size Size of the memory buffer in bytes.
 * @param[out] width Output parameter for the image width.
 * @param[out] height Output parameter for the image height.
 * @param[out] pixels Output vector to hold the pixel data (RGBA format).
 * @return 0 on success, non-zero on failure.
 */
int loadFromMemory(
    const unsigned char* buffer,
    int size,
    int& width,
    int& height,
    std::vector<unsigned char>& pixels
);
/**
 * @brief Write an RGBA image to a file in the specified format.
 * @param format The image format to use for saving (BMP, JPG, PNG, TGA).
 * @param filename The path to the output image file.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param pixels Pointer to the pixel data (RGBA format).
 * @param verticalFlip Whether to vertically flip the image before saving.
 * @return 0 on success, non-zero on failure.
 */
int writeToFile(
    Format format,
    const std::string& filename,
    int width,
    int height,
    const unsigned char* pixels,
    bool verticalFlip = false
);

} // namespace ImageRGBA
