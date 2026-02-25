/**
 * @file AppTextureManager.cpp
 * @brief Implementation of the AppTextureManager class.
 */

#include "app/AppTextureManager.h"

#include "utils/Logger.hpp"
#include "utils/Image.h"

void AppTextureManager::init(GfxRenderer renderer) {
    m_renderer = renderer;
    // Init the default texture
    std::vector<unsigned int> data = { 0x0, 0x0, 0x0, 0xFF }; // 1x1 black pixel
    GfxImageInfo info = {};
    info.width = 1;
    info.height = 1;
    info.format = GfxFormat::R8G8B8A8_UNORM;
    info.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
    m_defaultTexture = m_renderer->createImage(info);
    if (m_defaultTexture)
        m_renderer->setImageData(m_defaultTexture, data.data());
}

void AppTextureManager::term() {
    if (m_defaultTexture)
        m_renderer->destroyImage(m_defaultTexture);
    m_defaultTexture = nullptr;
    m_textures.clear();
    m_renderer = nullptr;
}

GfxImage AppTextureManager::getTexture(const std::string& filename) {
    if (!m_renderer || filename.empty())
        return nullptr;

    // Check if texture is already loaded
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        if (auto img = it->second.lock())
            return img;
        else
            m_textures.erase(it); // Remove expired weak_ptr
    }

    // Load image from file
    int width = 0, height = 0;
    std::vector<unsigned char> pixels;
    if (ImageRGBA::loadFromFile(filename, width, height, pixels)) {
        Logger() << "Failed to load texture: " << filename;
        return nullptr;
    }

    // Create GfxImage from pixel data
    GfxImageInfo info = {};
    info.width = width;
    info.height = height;
    info.format = GfxFormat::R8G8B8A8_UNORM;
    info.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
    GfxImage image = m_renderer->createImage(info);
    if (!image) {
        Logger() << "Failed to create GfxImage for texture: " << filename;
        return nullptr;
    }

    // Upload pixel data to the image
    if (m_renderer->setImageData(image, pixels.data())) {
        Logger() << "Failed to upload texture data for: " << filename;
        return nullptr;
    }

    m_textures[filename] = image;

    return image;
}

GfxImage AppTextureManager::getIntensityTexture(const std::string& filename) {
    if (!m_renderer || filename.empty())
        return nullptr;

    // Check if texture is already loaded
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        if (auto img = it->second.lock())
            return img;
        else
            m_textures.erase(it); // Remove expired weak_ptr
    }

    // Read data from file
    int width = 0, height = 0;
    std::ifstream file(filename);
    if (!file)
        return nullptr;
    std::vector<float> data{};
    data.reserve(1024);
    std::string line{};
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        float value = 0.0f;
        int currentWidth = 0;
        while (iss >> value) {
            data.push_back(value);
            currentWidth++;
        }
        if (height == 0)
            width = currentWidth;
        else if (currentWidth != width)
            return nullptr;
        height++;
    }
    if (width == 0 || height == 0)
        return nullptr;

    // Create GfxImage
    GfxImageInfo info = {};
    info.width = width;
    info.height = height;
    info.format = GfxFormat::R32_SFLOAT;
    info.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
    GfxImage image = m_renderer->createImage(info);
    if (!image) {
        Logger() << "Failed to create GfxImage for texture: " << filename;
        return nullptr;
    }

    // Upload pixel data to the image
    if (m_renderer->setImageData(image, data.data())) {
        Logger() << "Failed to upload texture data for: " << filename;
        return nullptr;
    }

    m_textures[filename] = image;

    return image;
}

GfxImage AppTextureManager::getIntensityPreviewTexture(const std::string& filename) {
    if (!m_renderer || filename.empty())
        return nullptr;

    // Check if texture is already loaded
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        if (auto img = it->second.lock())
            return img;
        else
            m_textures.erase(it); // Remove expired weak_ptr
    }

    // Read data from file
    int width = 0, height = 0;
    std::ifstream file(filename);
    if (!file)
        return nullptr;
    std::vector<float> data{};
    data.reserve(1024);
    float minValue = std::numeric_limits<float>::max();
    float maxValue = std::numeric_limits<float>::lowest();
    std::string line{};
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        float value = 0.0f;
        int currentWidth = 0;
        while (iss >> value) {
            data.push_back(value);
            minValue = std::min(minValue, value);
            maxValue = std::max(maxValue, value);
            currentWidth++;
        }
        if (height == 0)
            width = currentWidth;
        else if (currentWidth != width)
            return nullptr;
        height++;
    }
    if (width == 0 || height == 0)
        return nullptr;

    // Convert to RGBA8
    struct Color {
        float r, g, b;
    };
    auto lerp = [](const Color& a, const Color& b, float t) -> Color {
        return {
                a.r + (b.r - a.r) * t,
                a.g + (b.g - a.g) * t,
                a.b + (b.b - a.b) * t
        };
        };
    auto heatmap = [&lerp](float t) -> Color {
        t = std::clamp(t, 0.0f, 1.0f);
        const Color c0{ 0.0f, 0.0f, 0.5f }; // dark blue
        const Color c1{ 0.0f, 0.0f, 1.0f }; // blue
        const Color c2{ 0.0f, 1.0f, 1.0f }; // cyan
        const Color c3{ 0.0f, 1.0f, 0.0f }; // green
        const Color c4{ 1.0f, 1.0f, 0.0f }; // yellow
        const Color c5{ 1.0f, 0.0f, 0.0f }; // red
        if (t < 0.2f)
            return lerp(c0, c1, t / 0.2f);
        if (t < 0.4f)
            return lerp(c1, c2, (t - 0.2f) / 0.2f);
        if (t < 0.6f)
            return lerp(c2, c3, (t - 0.4f) / 0.2f);
        if (t < 0.8f)
            return lerp(c3, c4, (t - 0.6f) / 0.2f);
        return lerp(c4, c5, (t - 0.8f) / 0.2f);
        };
    std::vector<uint8_t> rgba(width * height * 4);
    const float range = maxValue - minValue;
    const bool validRange = range > std::numeric_limits<float>::epsilon();
    for (size_t i = 0; i < data.size(); ++i) {
        float t = 0.0f;
        if (validRange)
            t = (data[i] - minValue) / range;

        Color c = heatmap(t);

        size_t base = i * 4;
        rgba[base + 0] = static_cast<uint8_t>(c.r * 255.0f);
        rgba[base + 1] = static_cast<uint8_t>(c.g * 255.0f);
        rgba[base + 2] = static_cast<uint8_t>(c.b * 255.0f);
        rgba[base + 3] = 255;
    }

    // Create GfxImage
    GfxImageInfo info = {};
    info.width = width;
    info.height = height;
    info.format = GfxFormat::R8G8B8A8_UNORM;
    info.usages.set(GfxImageUsage::SAMPLED_TEXTURE);
    GfxImage image = m_renderer->createImage(info);
    if (!image) {
        Logger() << "Failed to create GfxImage for texture: " << filename;
        return nullptr;
    }

    // Upload pixel data to the image
    if (m_renderer->setImageData(image, rgba.data())) {
        Logger() << "Failed to upload texture data for: " << filename;
        return nullptr;
    }

    m_textures[filename] = image;

    return image;
}

GfxImage AppTextureManager::getDefaultTexture() {
    return m_defaultTexture;
}
