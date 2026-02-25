/**
 * @file Stopwatch.cpp
 * @brief Implementation of the Stopwatch class for measuring elapsed time.
 */

#include "utils/Stopwatch.h"

void Stopwatch::start() {
    if (!m_running) {
        m_startTime = std::chrono::steady_clock::now();
        m_running = true;
        m_paused = false;
        m_elapsedPaused = 0.0;
    } else if (m_paused) {
        auto duration = std::chrono::duration<double, std::micro>(m_elapsedPaused * 1000.0);
        m_startTime =
            std::chrono::steady_clock::now() -
            std::chrono::duration_cast<std::chrono::microseconds>(duration);
        m_paused = false;
    }
}

void Stopwatch::pause() {
    if (m_running && !m_paused) {
        auto now = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(now - m_startTime);
        m_elapsedPaused = duration.count() / 1000.0;
        m_paused = true;
    }
}

double Stopwatch::elapsed() const {
    if (!m_running)
        return 0.0;
    if (m_paused)
        return m_elapsedPaused;
    auto now = std::chrono::steady_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(now - m_startTime);
    return duration.count() / 1000.0;
}

void Stopwatch::reset() {
    m_running = false;
    m_paused = false;
    m_elapsedPaused = 0.0;
    m_startTime = std::chrono::steady_clock::now();
}

bool Stopwatch::isPaused() const {
    return m_paused;
}

bool Stopwatch::isRunning() const {
    return m_running;
}
