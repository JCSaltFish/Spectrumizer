/**
 * @file Stopwatch.h
 * @brief Header file for the Stopwatch class.
 */

#include "UtilsCommon.h"

#pragma once

/**
 * @brief Class for measuring elapsed time.
 */
class Stopwatch {
public:
    /**
     * @brief Starts or resumes the stopwatch.
     */
    void start();
    /**
     * @brief Pauses the stopwatch.
     */
    void pause();
    /**
     * @brief Returns the elapsed time in milliseconds since the stopwatch was started.
     * @return Elapsed time in milliseconds.
     */
    double elapsed() const;
    /**
     * @brief Resets the stopwatch to zero and stops it.
     */
    void reset();
    /**
     * @brief Checks if the stopwatch is currently paused.
     * @return True if paused, false otherwise.
     */
    bool isPaused() const;
    /**
     * @brief Checks if the stopwatch is running (either active or paused).
     * @return True if running, false otherwise.
     */
    bool isRunning() const;

private:
    std::chrono::steady_clock::time_point m_startTime; // Start time point
    bool m_running = false; // Stopwatch running state
    bool m_paused = false; // Whether the stopwatch is paused
    double m_elapsedPaused = 0.0; // Elapsed time when paused (in milliseconds)
};
