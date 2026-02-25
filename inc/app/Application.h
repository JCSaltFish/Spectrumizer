/*
 * Copyright 2025 Jed Wang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file Application.h
 * @brief Header file for the Application class.
 */

#pragma once

#include "app/AppConfig.h"
#include "app/AppDataManager.h"
#include "gui/GuiPub.h"

/**
 * @brief Class to manage application version information.
 */
class AppVersion {
public:
    /**
     * @brief Get the version string in the format "major.minor.patch".
     * @return Version string.
     */
    static const std::string getVersionString();
    /**
     * @brief Get the version number as an integer (major * 100 + minor * 10 + patch).
     * @return Version number.
     */
    static const int getVersionNumber();

public:
    static const int MAJOR; // Major version
    static const int MINOR; // Minor version
    static const int PATCH; // Patch version
};

/**
 * @brief Base class for application.
 */
class BaseApp {
public:
    BaseApp(int argc, char** argv);
    virtual ~BaseApp() = default;

    virtual int init() = 0;
    virtual int run() = 0;
    virtual void term() = 0;

protected:
    int m_argc = 0; // Argument count
    char** m_argv = nullptr; // Argument vector
};

/**
 * @brief Main application class that manages the application lifecycle.
 */
class Application {
public:
    Application(int argc, char** argv);
    ~Application() = default;

    /**
     * @brief Initializes the application.
     * @return 0 on success, non-zero on failure.
     */
    int init();
    /**
     * @brief Runs the application.
     * @return 0 on success, non-zero on failure.
     */
    int run();
    /**
     * @brief Terminates the application.
     */
    void term();

public:
    static const char* const APP_NAME; // Application name

private:
    std::unique_ptr<BaseApp> m_pApp; // App instance

    int m_argc = 0; // Argument count
    char** m_argv = nullptr; // Argument vector
};
