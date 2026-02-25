/**
 * @file PtMaterial.h
 * @brief Declaration of the PtMaterial class representing a material with various properties.
 */

#pragma once

#include "db/DbPub.h"
#include "utils/Math.h"
#include "utils/Flags.hpp"

/**
 * @brief Represents a material with various properties for rendering.
 */
class PtMaterial {
    friend class DbTypeRegistry;
    /* OBJECT TYPE INFO */
private:
    static void serialize(DbSerializer& serializer, const PtMaterial& material);
    static void deserialize(DbSerializer& serializer, PtMaterial& material);
    static void migrate(int oldVersion, PtMaterial& material);
public:
    static constexpr const char* TYPE_NAME = "PtMaterial";
    static constexpr int VERSION = 1;

    /**
     * @brief Flags representing material properties.
     */
    enum class MaterialFlag {
        NORMAL_MAP = 1 << 0, // Has normal texture
        ROUGHNESS_MAP = 1 << 1, // Has roughness texture
        TEMPERATURE_MAP = 1 << 2, // Has temperature texture (for spectral rendering)
        HIGHLIGHT = 1 << 3, // Highlighted material
    };

    /**
     * @brief Types of materials.
     */
    enum class MaterialType {
        DIFFUSE,
        SPECULAR,
        GLOSSY,
        TRANSLUCENT,
    };

    /* OBJECT FIELDS */
private:
    DB::ID m_meshId = 0; // ID of the associated mesh

    int m_type = 0; // Material type

    float m_roughness = 1.0f; // Surface roughness
    float m_ior = 1.5f; // Index of refraction
    float m_temperature = 0.0f; // Temperature in Celsius (for spectral rendering)

    uint32_t m_flags = 0; // Material flags (bitmask)

    DbFilePath m_normalTexPath; // Path to normal map texture
    DbFilePath m_roughnessTexPath; // Path to roughness texture
    DbFilePath m_temperatureTexPath; // Path to temperature texture

    DB::ID m_spMaterialId = 0; // ID of the associated spectrum material (for spectral rendering)

    /* OBJECT METHODS */
private:
    /**
     * @brief Get a const pointer to the PtMaterial object from a handle.
     * @param hMaterial Handle to the material object.
     * @return Pointer to the PtMaterial object, or nullptr if invalid.
     */
    static const PtMaterial* view(const DbObjHandle& hMaterial);
public:
    /**
     * @brief Get the associated mesh of the material.
     * @param hMaterial Handle to the material object.
     * @return Handle to the associated mesh, or invalid handle if none.
     */
    static DbObjHandle getMesh(const DbObjHandle& hMaterial);
    /**
     * @brief Set the associated mesh of the material.
     * @param hMaterial Handle to the material object.
     * @param hMesh Handle to the new mesh object.
     * @return Result code indicating success or failure.
     */
    static DB::Result setMesh(const DbObjHandle& hMaterial, const DbObjHandle& hMesh);
    /**
     * @brief Get the type of the material.
     * @param hMaterial Handle to the material object.
     * @return MaterialType enum value, or MaterialType::DIFFUSE if invalid.
     */
    static MaterialType getType(const DbObjHandle& hMaterial);
    /**
     * @brief Set the type of the material.
     * @param hMaterial Handle to the material object.
     * @param type New MaterialType enum value.
     * @return Result code indicating success or failure.
     */
    static DB::Result setType(const DbObjHandle& hMaterial, MaterialType type);
    /**
     * @brief Get the roughness of the material.
     * @param hMaterial Handle to the material object.
     * @return Roughness value, or 0.0f if invalid.
     */
    static float getRoughness(const DbObjHandle& hMaterial);
    /**
     * @brief Set the roughness of the material.
     * @param hMaterial Handle to the material object.
     * @param roughness New roughness value.
     * @return Result code indicating success or failure.
     */
    static DB::Result setRoughness(const DbObjHandle& hMaterial, float roughness);
    /**
     * @brief Get the index of refraction (IOR) of the material.
     * @param hMaterial Handle to the material object.
     * @return IOR value, or 0.0f if invalid.
     */
    static float getIOR(const DbObjHandle& hMaterial);
    /**
     * @brief Set the index of refraction (IOR) of the material.
     * @param hMaterial Handle to the material object.
     * @param ior New IOR value.
     * @return Result code indicating success or failure.
     */
    static DB::Result setIOR(const DbObjHandle& hMaterial, float ior);
    /**
     * @brief Get the temperature of the material in Celsius.
     * @param hMaterial Handle to the material object.
     * @return Temperature in Celsius, or 0.0f if invalid.
     */
    static float getTemperature(const DbObjHandle& hMaterial);
    /**
     * @brief Set the temperature of the material in Celsius.
     * @param hMaterial Handle to the material object.
     * @param temperature New temperature in Celsius.
     * @return Result code indicating success or failure.
     */
    static DB::Result setTemperature(const DbObjHandle& hMaterial, float temperature);
    /**
     * @brief Get the material flags.
     * @param hMaterial Handle to the material object.
     * @return Flags as a bitmask, or 0 if invalid.
     */
    static Flags<MaterialFlag> getFlags(const DbObjHandle& hMaterial);
    /**
     * @brief Set the material flags.
     * @param hMaterial Handle to the material object.
     * @param flags New flags as a bitmask.
     * @return Result code indicating success or failure.
     */
    static DB::Result setFlags(const DbObjHandle& hMaterial, Flags<MaterialFlag> flags);
    /**
     * @brief Get the file path of the normal map texture.
     * @param hMaterial Handle to the material object.
     * @return File path as a string, or empty string if invalid.
     */
    static std::string getNormalTexPath(const DbObjHandle& hMaterial);
    /**
     * @brief Set the file path of the normal map texture.
     * @param hMaterial Handle to the material object.
     * @param path New file path as a string.
     * @return Result code indicating success or failure.
     */
    static DB::Result setNormalTexPath(const DbObjHandle& hMaterial, const std::string& path);
    /**
     * @brief Get the file path of the roughness map texture.
     * @param hMaterial Handle to the material object.
     * @return File path as a string, or empty string if invalid.
     */
    static std::string getRoughnessTexPath(const DbObjHandle& hMaterial);
    /**
     * @brief Set the file path of the roughness map texture.
     * @param hMaterial Handle to the material object.
     * @param path New file path as a string.
     * @return Result code indicating success or failure.
     */
    static DB::Result setRoughnessTexPath(const DbObjHandle& hMaterial, const std::string& path);
    /**
     * @brief Get the file path of the temperature map texture.
     * @param hMaterial Handle to the material object.
     * @return File path as a string, or empty string if invalid.
     */
    static std::string getTemperatureTexPath(const DbObjHandle& hMaterial);
    /**
     * @brief Set the file path of the temperature map texture.
     * @param hMaterial Handle to the material object.
     * @param path New file path as a string.
     * @return Result code indicating success or failure.
     */
    static DB::Result setTemperatureTexPath(const DbObjHandle& hMaterial, const std::string& path);
    /**
     * @brief Get the associated spectrum material of the material (for spectral rendering).
     * @param hMaterial Handle to the material object.
     * @return Handle to the associated spectrum material, or invalid handle if none.
     */
    static DbObjHandle getSpectrumMaterial(const DbObjHandle& hMaterial);
    /**
     * @brief Set the associated spectrum material of the material (for spectral rendering).
     * @param hMaterial Handle to the material object.
     * @param hSpMaterial Handle to the spectrum material object to associate.
     * @return Result code indicating success or failure.
     */
    static DB::Result setSpectrumMaterial(const DbObjHandle& hMaterial, const DbObjHandle& hSpMaterial);
    /**
     * @brief Copy the material object to another database.
     * @param hMaterial Handle to the material object to copy.
     * @param dst Pointer to the destination database.
     * @return Handle to the copied material in the destination database.
     */
    static DbObjHandle copy(const DbObjHandle& hMaterial, std::shared_ptr<DB>& dst);
};
