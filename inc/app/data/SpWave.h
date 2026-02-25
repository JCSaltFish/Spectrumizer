/**
 * @file SpWave.h
 * @brief Declaration of the SpWave class representing a wave for spectral rendering.
 */

#pragma once

#include "db/DbPub.h"

/**
 * @brief Represents a wave for spectral rendering.
 */
class SpWave {
    friend class DbTypeRegistry;
    /* OBJECT TYPE INFO */
private:
    static void serialize(DbSerializer& serializer, const SpWave& wave);
    static void deserialize(DbSerializer& serializer, SpWave& wave);
    static void migrate(int oldVersion, SpWave& wave);
public:
    static constexpr const char* TYPE_NAME = "SpWave";
    static constexpr int VERSION = 1;

    /* OBJECT FIELDS */
private:
    float m_waveNumber = 0.0f; // Wave number in inverse centimeters (cm^-1)

    /* OBJECT METHODS */
private:
    /**
     * @brief Get a const pointer to the SpWave object from a handle.
     * @param hWave Handle to the wave object.
     * @return Pointer to the SpWave object, or nullptr if invalid.
     */
    static const SpWave* view(const DbObjHandle& hWave);
public:
    /**
     * @brief Get the wave number.
     * @param hWave Handle to the wave object.
     * @return The wave number, or 0.0f if invalid.
     */
    static float getWaveNumber(const DbObjHandle& hWave);
    /**
     * @brief Set the wave number.
     * @param hWave Handle to the wave object.
     * @param waveNum New wave number value.
     * @return Result code indicating success or failure.
     */
    static DB::Result setWaveNumber(const DbObjHandle& hWave, float waveNum);
};
