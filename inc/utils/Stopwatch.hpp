/**
 * @file Stopwatch.hpp
 * @brief Header file for the Stopwatch class.
 */

#include "UtilsCommon.h"

#pragma once

 /**
  * @brief Class for measuring elapsed time.
  */
class Stopwatch {
public:
    /*
     * @brief Starts or resumes the stopwatch.
     */
    void start() {
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
    };
    /*
     * @brief Pauses the stopwatch.
     */
    void pause() {
        if (m_running && !m_paused) {
            auto now = std::chrono::steady_clock::now();
            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(now - m_startTime);
            m_elapsedPaused = duration.count() / 1000.0;
            m_paused = true;
        }
    };
    /*
     * @brief Returns the elapsed time in milliseconds since the stopwatch was started.
     * @return Elapsed time in milliseconds.
     */
    double elapsed() const {
        if (!m_running)
            return 0.0;
        if (m_paused)
            return m_elapsedPaused;
        auto now = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(now - m_startTime);
        return duration.count() / 1000.0;
    };
    /*
     * @brief Resets the stopwatch to zero and stops it.
     */
    void reset() {
        m_running = false;
        m_paused = false;
        m_elapsedPaused = 0.0;
        m_startTime = std::chrono::steady_clock::now();
    };
    /*
     * @brief Checks if the stopwatch is currently paused.
     * @return True if paused, false otherwise.
     */
    bool isPaused() const {
        return m_paused;
    };
    /*
     * @brief Checks if the stopwatch is running (either active or paused).
     * @return True if running, false otherwise.
     */
    bool isRunning() const {
        return m_running;
    };

private:
    std::chrono::steady_clock::time_point m_startTime; // Start time point
    bool m_running = false; // Stopwatch running state
    bool m_paused = false; // Whether the stopwatch is paused
    double m_elapsedPaused = 0.0; // Elapsed time when paused (in milliseconds)
};
