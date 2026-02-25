/**
 * @file Application.cpp
 * @brief Implementation of the Application class.
 */

#include "app/Application.h"

#include "app/PathTracerApp.h"

using App = PathTracerApp;

const char* const Application::APP_NAME = "Spectrumizer";

const int AppVersion::MAJOR = 3;
const int AppVersion::MINOR = 0;
const int AppVersion::PATCH = 0;

constexpr const char* CONFIG_FILE = "config";

const std::string AppVersion::getVersionString() {
    return std::to_string(MAJOR) + "." + std::to_string(MINOR) + "." + std::to_string(PATCH);
}

const int AppVersion::getVersionNumber() {
    return MAJOR * 100 + MINOR * 10 + PATCH;
}

BaseApp::BaseApp(int argc, char** argv) :
    m_argc(argc),
    m_argv(argv) {}

Application::Application(int argc, char** argv) :
    m_argc(argc),
    m_argv(argv) {
    m_pApp = std::make_unique<App>(argc, argv);
}

int Application::init() {
    AppConfig::instance().init(APP_NAME, CONFIG_FILE);
    AppDataManager::instance().init();
    return m_pApp->init();
}

int Application::run() {
    return m_pApp->run();
}

void Application::term() {
    m_pApp->term();
}
