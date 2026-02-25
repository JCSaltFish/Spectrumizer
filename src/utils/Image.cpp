/**
 * @file Image.cpp
 * @brief Implementation of the ImageRGBA utility.
 */

#include "utils/Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int ImageRGBA::loadFromFile(
    const std::string& filename,
    int& width,
    int& height,
    std::vector<unsigned char>& pixels
) {
    int n;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &n, 4);
    if (data == nullptr)
        return 1;
    size_t w = static_cast<size_t>(width);
    size_t h = static_cast<size_t>(height);
    pixels.resize(w * h * 4);
    std::memcpy(pixels.data(), data, w * h * 4);
    stbi_image_free(data);
    return 0;
}

int ImageRGBA::loadFromMemory(
    const unsigned char* buffer,
    int size,
    int& width,
    int& height,
    std::vector<unsigned char>& pixels
) {
    int n;
    unsigned char* data = stbi_load_from_memory(buffer, size, &width, &height, &n, 4);
    if (data == nullptr)
        return 1;
    size_t w = static_cast<size_t>(width);
    size_t h = static_cast<size_t>(height);
    pixels.resize(w * h * 4);
    std::memcpy(pixels.data(), data, w * h * 4);
    stbi_image_free(data);
    return 0;
}

int ImageRGBA::writeToFile(
    Format format,
    const std::string& filename,
    int width,
    int height,
    const unsigned char* pixels,
    bool verticalFlip
) {
    int result = 0;
    stbi_flip_vertically_on_write(verticalFlip ? 1 : 0);
    if (format == Format::BMP)
        result = stbi_write_bmp(filename.c_str(), width, height, 4, pixels);
    else if (format == Format::JPG)
        result = stbi_write_jpg(filename.c_str(), width, height, 4, pixels, 100);
    else if (format == Format::PNG)
        result = stbi_write_png(filename.c_str(), width, height, 4, pixels, width * 4);
    else if (format == Format::TGA)
        result = stbi_write_tga(filename.c_str(), width, height, 4, pixels);
    if (result == 0)
        return 1;
    return 0;
}
