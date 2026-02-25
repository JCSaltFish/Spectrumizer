# Copyright 2025 Jed Wang
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.15)
project(Spectrumizer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX "_d")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
set(GLFW_ROOT ${THIRD_PARTY_DIR}/glfw)
set(STB_INCLUDE_DIR ${THIRD_PARTY_DIR}/stb)
set(TINYFILEDIALOGS_DIR ${THIRD_PARTY_DIR}/tinyfiledialogs)
set(GLAD_DIR ${THIRD_PARTY_DIR}/glad)
set(IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
set(FONTS_DIR ${THIRD_PARTY_DIR}/fonts)

if(NOT DEFINED VULKAN_SDK_PATH)
    set(VULKAN_SDK_PATH "${THIRD_PARTY_DIR}/vulkan")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

add_subdirectory(${GLFW_ROOT})
set(GLFW_LIBRARY glfw)
set_target_properties(glfw PROPERTIES FOLDER "Dependencies")
if(TARGET uninstall)
    set_target_properties(uninstall PROPERTIES FOLDER "Dependencies")
endif()
if(TARGET update_mappings)
    set_target_properties(update_mappings PROPERTIES FOLDER "Dependencies")
endif()

add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
set_target_properties(glad PROPERTIES FOLDER "Dependencies")
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

if(EXISTS ${IMGUI_DIR}/CMakeLists.txt)
    add_subdirectory(${IMGUI_DIR})
    set_target_properties(imgui PROPERTIES FOLDER "Dependencies")
else()
    file(GLOB IMGUI_SOURCES 
        ${IMGUI_DIR}/*.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
    )
    add_library(imgui STATIC ${IMGUI_SOURCES})
    set_target_properties(imgui PROPERTIES FOLDER "Dependencies")
    target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${VULKAN_SDK_PATH}/include
    )
    target_link_libraries(imgui ${GLFW_LIBRARY})
endif()

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_INCLUDE_DIR})

add_library(tinyfiledialogs STATIC ${TINYFILEDIALOGS_DIR}/tinyfiledialogs.c)
set_target_properties(tinyfiledialogs PROPERTIES FOLDER "Dependencies")
target_include_directories(tinyfiledialogs PUBLIC ${TINYFILEDIALOGS_DIR})

add_library(json INTERFACE)
target_include_directories(json INTERFACE ${THIRD_PARTY_DIR})

add_library(fonts STATIC
    ${FONTS_DIR}/ForkAwesome.c
    ${FONTS_DIR}/SourceSansPro.c
    ${FONTS_DIR}/SourceHanSansSC.c
)
set_target_properties(fonts PROPERTIES FOLDER "Dependencies")
target_include_directories(fonts PUBLIC ${FONTS_DIR})

if(WIN32)
    set(VULKAN_LIB_NAME vulkan-1)
else()
    set(VULKAN_LIB_NAME vulkan)
endif()
find_library(Vulkan_LIB
    NAMES ${VULKAN_LIB_NAME}
    PATHS "${VULKAN_SDK_PATH}/Lib"
    REQUIRED
)

if(WIN32)
    set(GLSLANG_LIB_DIR "${VULKAN_SDK_PATH}/Lib")
    set(LIB_EXT ".lib")
else()
    set(GLSLANG_LIB_DIR "${VULKAN_SDK_PATH}/lib")
    set(LIB_EXT ".a")
endif()
set(GLSLANG_LIBS
    GenericCodeGen
    glslang
    glslang-default-resource-limits
    MachineIndependent
    SPIRV
    SPIRV-Tools-opt
    SPIRV-Tools
)
foreach(LIB IN LISTS GLSLANG_LIBS)
    if(WIN32)
        set(LIB_PATH 
            "$<$<OR:$<CONFIG:Debug>,$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>>:${GLSLANG_LIB_DIR}/${LIB}d${LIB_EXT}>"
            "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<NOT:$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>>>:${GLSLANG_LIB_DIR}/${LIB}${LIB_EXT}>"
        )
        list(APPEND FULL_GLSLANG_LIBS ${LIB_PATH})
    else()
        list(APPEND FULL_GLSLANG_LIBS "${GLSLANG_LIB_DIR}/lib${LIB}${LIB_EXT}")
    endif()
endforeach()
if(UNIX AND NOT APPLE)
    list(APPEND FULL_GLSLANG_LIBS "${GLSLANG_LIB_DIR}/libSPIRV-Tools-shared.so")
endif()

file(GLOB_RECURSE SOURCES
    "inc/*.h"
    "inc/*.hpp"
    "src/*.cpp"
)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/inc
    ${GLFW_ROOT}/include
    ${STB_INCLUDE_DIR}
    ${TINYFILEDIALOGS_DIR}
    ${GLAD_DIR}/include
    ${VULKAN_SDK_PATH}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${FONTS_DIR}
)

target_link_libraries(${PROJECT_NAME}
    ${GLFW_LIBRARY}
    ${Vulkan_LIB}
    ${FULL_GLSLANG_LIBS}
    glad
    imgui
    stb
    tinyfiledialogs
    json
    fonts
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        opengl32.lib
        Shell32
        Ole32
    )
elseif(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME}
        OpenGL::GL
        Threads::Threads
        dl
    )
endif()

set(LANG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/strings")
set(LANG_LIST
    en_US
    zh_CN
)
if (UNIX AND NOT APPLE)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/strings)
    foreach(LANG ${LANG_LIST})
        set(OBJ ${CMAKE_BINARY_DIR}/strings/${LANG}.o)

        add_custom_command(
            OUTPUT ${OBJ}
            COMMAND ${CMAKE_LINKER}
                -r -b binary
                ${LANG}.json
                -o ${OBJ}
            WORKING_DIRECTORY ${LANG_DIR}
            DEPENDS ${LANG_FILE}
            COMMENT "Embedding ${LANG}.json"
        )

        list(APPEND LANG_OBJECTS ${OBJ})
    endforeach()

    add_custom_target(LangStrings ALL DEPENDS ${LANG_OBJECTS})
    target_sources(${PROJECT_NAME} PRIVATE ${LANG_OBJECTS})
    add_dependencies(${PROJECT_NAME} LangStrings)
elseif (MSVC)
    set(RC_FILE ${CMAKE_BINARY_DIR}/strings/strings.rc)
    file(WRITE ${RC_FILE} "/**\n * @file strings.rc\n")
    file(APPEND ${RC_FILE} " * @brief Auto-generated language string resource file.\n */\n\n")

    foreach(LANG ${LANG_LIST})
        string(TOUPPER ${LANG} VAR_NAME)
        file(APPEND ${RC_FILE}
            "LANG_${VAR_NAME}_JSON RCDATA \"${LANG_DIR}/${LANG}.json\"\n"
        )
    endforeach()

    add_library(LangStrings OBJECT ${RC_FILE})
    target_link_libraries(${PROJECT_NAME} LangStrings)
endif()

set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders")
set(SHADER_OUTPUT_HPP "${CMAKE_CURRENT_SOURCE_DIR}/inc/res/ShaderStrings.hpp")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/inc/res")
file(GLOB_RECURSE SHADER_FILES 
    LIST_DIRECTORIES false 
    "${SHADER_DIR}/*.glsl"
    "${SHADER_DIR}/*.vert"
    "${SHADER_DIR}/*.frag"
    "${SHADER_DIR}/*.comp"
)
add_custom_command(OUTPUT ${SHADER_OUTPUT_HPP}
    COMMAND ${CMAKE_COMMAND} 
        -D "INPUT_DIR=${SHADER_DIR}"
        -D "OUTPUT_HPP=${SHADER_OUTPUT_HPP}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateShaderStrings.cmake"
    DEPENDS 
        "${SHADER_FILES}"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateShaderStrings.cmake"
    COMMENT "Generating ShaderStrings.hpp"
)
set_source_files_properties(${SHADER_OUTPUT_HPP} PROPERTIES GENERATED TRUE)
add_custom_target(GenerateShaderStrings DEPENDS ${SHADER_OUTPUT_HPP})
add_dependencies(${PROJECT_NAME} GenerateShaderStrings)

file(GLOB LANG_FILES "${LANG_DIR}/*.json")
foreach(LANG_FILE ${LANG_FILES})
    file(RELATIVE_PATH RELATIVE_PATH "${CMAKE_SOURCE_DIR}" "${LANG_FILE}")
    get_filename_component(GROUP_PATH "${RELATIVE_PATH}" DIRECTORY)
    string(REPLACE "/" "\\" GROUP_NAME "${GROUP_PATH}")
    source_group("${GROUP_NAME}" FILES "${LANG_FILE}")
endforeach()
target_sources(${PROJECT_NAME} PRIVATE ${LANG_FILES})
set_source_files_properties(${LANG_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

file(GLOB_RECURSE SHADER_FILES 
    "${SHADER_DIR}/*.glsl"
    "${SHADER_DIR}/*.vert"
    "${SHADER_DIR}/*.frag"
    "${SHADER_DIR}/*.comp"
)
foreach(SHADER_FILE ${SHADER_FILES})
    file(RELATIVE_PATH RELATIVE_PATH "${CMAKE_SOURCE_DIR}" "${SHADER_FILE}")
    get_filename_component(GROUP_PATH "${RELATIVE_PATH}" DIRECTORY)
    string(REPLACE "/" "\\" GROUP_NAME "${GROUP_PATH}")
    source_group("${GROUP_NAME}" FILES "${SHADER_FILE}")
endforeach()
target_sources(${PROJECT_NAME} PRIVATE ${SHADER_FILES})
set_source_files_properties(${SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE resources/rc/resource.rc)
    source_group("resources\\rc" FILES "resources/rc/resource.rc")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

foreach(FILE ${SOURCES})
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" GROUP "${PARENT_DIR}")
    source_group("${GROUP}" FILES "${FILE}")
endforeach()

if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
endif()
