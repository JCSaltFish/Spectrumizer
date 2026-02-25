/**
 * @file FrameTimer.h
 * @brief Header file for the FrameTimer class.
 */

#pragma once

#include "UtilsCommon.h"

/**
 * @brief Class for measuring frame timing and performance.
 */
class FrameTimer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Duration = std::chrono::duration<double, std::milli>;

    FrameTimer() :
        m_lastFrameStart(Clock::now()),
        m_currentFrameStart(Clock::now()) {};

    /**
     * @brief Mark the beginning of a new frame.
     */
    void beginFrame();
    /**
     * @brief Mark the end of the current frame and calculate its duration.
     */
    void endFrame();

    /**
     * @brief Get the duration of the current frame.
     * @return The frame duration in milliseconds.
     */
    double getFrameDuration() const;
    /**
     * @brief Get the interval between the last and current frames.
     * @return The frame interval in milliseconds.
     */
    double getFrameInterval() const;
    /**
     * @brief Get the current frames per second (FPS).
     * @return The current FPS.
     */
    double getFPS() const;

private:
    TimePoint m_lastFrameStart; // Start time of the last frame
    TimePoint m_currentFrameStart; // Start time of the current frame
    Duration m_frameDuration = Duration::zero(); // Duration of the current frame
    Duration m_frameInterval = Duration::zero(); // Interval between the last and current frames
};
