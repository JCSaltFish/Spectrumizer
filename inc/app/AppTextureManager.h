/**
 * @file AppTextureManager.h
 * @brief Header file for the AppTextureManager class.
 */

#include "gfx/GfxPub.h"

/**
 * @brief Class for managing texture loading and caching.
 */
class AppTextureManager {
private:
    AppTextureManager() = default;
    ~AppTextureManager() = default;
    AppTextureManager(const AppTextureManager&) = delete;
    AppTextureManager& operator=(const AppTextureManager&) = delete;
    AppTextureManager(AppTextureManager&&) = delete;
    AppTextureManager& operator=(AppTextureManager&&) = delete;

public:
    static AppTextureManager& instance() {
        static AppTextureManager instance;
        return instance;
    };

    /**
     * @brief Initialize the texture manager with a graphics renderer.
     * @param renderer The graphics renderer to use.
     */
    void init(GfxRenderer renderer);
    /**
     * @brief Terminate the texture manager and release resources.
     */
    void term();

    /**
     * @brief Load a texture from a file and cache it.
     * @param filename Path to the texture file.
     * @return The loaded texture (GfxImage).
     */
    GfxImage getTexture(const std::string& filename);
    /**
     * @brief Load an intensity texture from a file and cache it.
     * @param filename Path to the intensity texture file.
     * @return The loaded intensity texture (GfxImage).
     */
    GfxImage getIntensityTexture(const std::string& filename);
    /**
     * @brief Load an intensity preview texture from a file and cache it.
     * @param filename Path to the intensity preview texture file.
     * @return The loaded intensity preview texture (GfxImage).
     */
    GfxImage getIntensityPreviewTexture(const std::string& filename);
    /**
     * @brief Get the default texture.
     * @return The default texture (GfxImage).
     */
    GfxImage getDefaultTexture();
    /**
     * @brief Clear the texture cache.
     */
    void clearCache() {
        m_textures.clear();
    };

private:
    GfxRenderer m_renderer = nullptr;
    std::unordered_map<std::string, std::weak_ptr<GfxImage_T>> m_textures; // Cache of textures
    GfxImage m_defaultTexture = nullptr; // Default texture
};
