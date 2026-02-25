/**
 * @file SpMaterial.h
 * @brief Declaration of the SpMaterial class representing a material for spectral rendering.
 */

#pragma once

#include "db/DbPub.h"

/**
 * @brief Represents a material for spectral rendering.
 */
class SpMaterial {
    friend class DbTypeRegistry;
    /* OBJECT TYPE INFO */
private:
    static void serialize(DbSerializer& serializer, const SpMaterial& material);
    static void deserialize(DbSerializer& serializer, SpMaterial& material);
    static void migrate(int oldVersion, SpMaterial& material);
public:
    static constexpr const char* TYPE_NAME = "SpMaterial";
    static constexpr int VERSION = 1;

    /* OBJECT FIELDS */
private:
    std::string m_name; // Name of the material
    std::vector<float> m_emissivities; // Emissivity values for each wave in the spectrum

    /* OBJECT METHODS */
private:
    static const SpMaterial* view(const DbObjHandle& hMaterial);
public:
    /**
     * @brief Get the name of the material.
     * @param hMaterial Handle to the material object.
     * @return The name of the material, or empty string if invalid.
     */
    static std::string getName(const DbObjHandle& hMaterial);
    /**
     * @brief Set the name of the material.
     * @param hMaterial Handle to the material object.
     * @param name New name for the material.
     * @return Result code indicating success or failure.
     */
    static DB::Result setName(const DbObjHandle& hMaterial, const std::string& name);
    /**
     * @brief Get the emissivity values of the material.
     * @param hMaterial Handle to the material object.
     * @return Vector of emissivity values, or empty vector if invalid.
     */
    static std::vector<float> getEmissivities(const DbObjHandle& hMaterial);
    /**
     * @brief Set the emissivity values of the material.
     * @param hMaterial Handle to the material object.
     * @param emissivities New vector of emissivity values for the material.
     * @return Result code indicating success or failure.
     */
    static DB::Result setEmissivities(
        const DbObjHandle& hMaterial,
        const std::vector<float>& emissivities
    );
};
