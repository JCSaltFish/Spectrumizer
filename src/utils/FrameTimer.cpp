/**
 * @file FrameTimer.cpp
 * @brief Implementation of the FrameTimer class.
 */

#include "utils/FrameTimer.h"

void FrameTimer::beginFrame() {
    m_lastFrameStart = m_currentFrameStart;
    m_currentFrameStart = Clock::now();
    if (m_lastFrameStart != m_currentFrameStart)
        m_frameInterval = m_currentFrameStart - m_lastFrameStart;
}

void FrameTimer::endFrame() {
    TimePoint frameEnd = Clock::now();
    m_frameDuration = frameEnd - m_currentFrameStart;
}

double FrameTimer::getFrameDuration() const {
    return m_frameDuration.count();
}

double FrameTimer::getFrameInterval() const {
    return m_frameInterval.count();
}

double FrameTimer::getFPS() const {
    if (m_frameInterval.count() > 0.0)
        return 1000.0 / m_frameInterval.count();
    return 0.0;
}
