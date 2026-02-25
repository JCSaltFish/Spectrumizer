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
 * @file main.cpp
 * @brief Main entry point for the application.
 */

#include "app/Application.h"

int main(int argc, char* argv[]) {
    Application app(argc, argv);
    if (app.init() != 0)
        return -1; // Initialization failed
    if (app.run() != 0) {
        app.term(); // Ensure proper termination on run failure
        return -1; // Run failed
    }
    app.term();

    return 0;
}
