/**
 * @file SpWave.cpp
 * @brief Implementation of the SpWave class representing a wave for spectral rendering.
 */

#include "app/AppDataManager.h"

void SpWave::serialize(DbSerializer& serializer, const SpWave& wave) {
    serializer.serialize(wave.m_waveNumber);
}

void SpWave::deserialize(DbSerializer &serializer, SpWave &wave) {
    serializer.deserialize(wave.m_waveNumber);
}

void SpWave::migrate(int oldVersion, SpWave &wave) {}

const SpWave *SpWave::view(const DbObjHandle &hWave) {
    if (!hWave.isValid() || hWave.getType() != SpWave::TYPE_NAME)
        return nullptr;
    return hWave.getDB()->objGet<SpWave>(hWave);
}

float SpWave::getWaveNumber(const DbObjHandle &hWave) {
    const SpWave* wave = view(hWave);
    if (!wave)
        return 0.0f;
    return wave->m_waveNumber;
}

DB::Result SpWave::setWaveNumber(const DbObjHandle &hWave, float waveNum) {
    const SpWave* wave = view(hWave);
    if (!wave)
        return DB::Result::INVALID_HANDLE;
    if (wave->m_waveNumber == waveNum)
        return DB::Result::SUCCESS;
    SpWave newWave = *wave;
    newWave.m_waveNumber = waveNum;
    return hWave.getDB()->objModify(hWave, newWave);
}
