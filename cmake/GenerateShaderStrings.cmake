# GenerateShaderStrings.cmake
# This CMake script generates a header file containing shader source code as string literals.

file(GLOB_RECURSE SHADER_FILES 
    LIST_DIRECTORIES false 
    RELATIVE "${INPUT_DIR}" 
    "${INPUT_DIR}/*.glsl"
    "${INPUT_DIR}/*.vert"
    "${INPUT_DIR}/*.frag"
    "${INPUT_DIR}/*.comp"
)

set(HPP_CONTENT "/**\n")
set(HPP_CONTENT "${HPP_CONTENT} * @file ShaderStrings.hpp\n")
set(HPP_CONTENT "${HPP_CONTENT} * @brief Auto-generated shader strings - DO NOT EDIT.\n")
set(HPP_CONTENT "${HPP_CONTENT} */\n\n")
set(HPP_CONTENT "${HPP_CONTENT}#pragma once\n\n")
set(HPP_CONTENT "${HPP_CONTENT}#include <string>\n")
set(HPP_CONTENT "${HPP_CONTENT}#include <unordered_map>\n\n")

set(HPP_CONTENT "${HPP_CONTENT}namespace ShaderStrings {\n\n")

foreach(SHADER_FILE ${SHADER_FILES})
    string(MAKE_C_IDENTIFIER "${SHADER_FILE}" VAR_NAME)
    string(REPLACE "/" "_" VAR_NAME "${VAR_NAME}")
    string(TOUPPER "${VAR_NAME}" VAR_NAME)

    file(READ "${INPUT_DIR}/${SHADER_FILE}" FILE_CONTENT)
    string(REPLACE "\"" "\\\"" ESCAPED_CONTENT "${FILE_CONTENT}")
    string(REGEX REPLACE "\n" "\\\\n\"\n    \"" ESCAPED_CONTENT "${ESCAPED_CONTENT}")

    set(HPP_CONTENT "${HPP_CONTENT}// Source: ${SHADER_FILE}\n")
    set(HPP_CONTENT "${HPP_CONTENT}inline constexpr const char* ${VAR_NAME} =\n")
    set(HPP_CONTENT "${HPP_CONTENT}    \"${ESCAPED_CONTENT}\";\n\n")
endforeach()

set(HPP_CONTENT "${HPP_CONTENT}/**\n")
set(HPP_CONTENT "${HPP_CONTENT} * @brief Retrieve the shader string by name.\n")
set(HPP_CONTENT "${HPP_CONTENT} * @param name The name of the shader file.\n")
set(HPP_CONTENT "${HPP_CONTENT} * @return The shader source code as a string, or an empty string if not found.\n")
set(HPP_CONTENT "${HPP_CONTENT} */\n")
set(HPP_CONTENT "${HPP_CONTENT}inline std::string get(const std::string& name) {\n")
set(HPP_CONTENT "${HPP_CONTENT}    static const auto map = std::unordered_map<std::string, std::string> {\n")
foreach(SHADER_FILE ${SHADER_FILES})
    string(MAKE_C_IDENTIFIER "${SHADER_FILE}" VAR_NAME)
    string(REPLACE "/" "_" VAR_NAME "${VAR_NAME}")
    string(TOUPPER "${VAR_NAME}" VAR_NAME)
    set(HPP_CONTENT "${HPP_CONTENT}        { \"${SHADER_FILE}\", ${VAR_NAME} },\n")
endforeach()
set(HPP_CONTENT "${HPP_CONTENT}    };\n")
set(HPP_CONTENT "${HPP_CONTENT}    if (auto it = map.find(name); it != map.end())\n")
set(HPP_CONTENT "${HPP_CONTENT}        return it->second;\n")
set(HPP_CONTENT "${HPP_CONTENT}    return {};\n}\n\n")

set(HPP_CONTENT "${HPP_CONTENT}} // namespace ShaderStrings\n")

file(WRITE "${OUTPUT_HPP}" "${HPP_CONTENT}")
message(STATUS "Generated shader strings: ${OUTPUT_HPP}")
