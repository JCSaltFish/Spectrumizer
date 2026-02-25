/**
 * @file ShaderStringsUtils.hpp
 * @brief Utility for resolving #include directives in shader strings.
 */

#pragma once

#include <set>
#include <regex>
#include <sstream>
#include <vector>

#include "ShaderStrings.hpp"

namespace ShaderStrings {

/**
 * @brief Class to resolve #include directives in shader strings.
 */
class IncludeResolver {
public:
    /**
     * @brief Resolves #include directives in the given shader file.
     * @param filePath The path of the shader file to resolve.
     * @return The resolved shader string with all includes processed.
     */
    std::string resolve(const std::string& filePath) {
        m_processingFiles.clear();
        return resolveImpl(filePath);
    }

private:
    /**
     * @brief Internal implementation to resolve includes recursively.
     * @param filePath The path of the shader file to resolve.
     * @param baseLineNumber The base line number for line directives.
     * @return The resolved shader string.
     */
    std::string resolveImpl(const std::string& filePath, int baseLineNumber = 1) {
        // Detect circular dependencies
        if (m_processingFiles.find(filePath) != m_processingFiles.end())
            return "// Circular dependency detected for: " + filePath + "\n";
        m_processingFiles.insert(filePath);

        try {
            std::string content = ShaderStrings::get(filePath);
            std::regex includePattern("#include\\s*\"([^\"]+)\"");
            std::smatch match;

            std::ostringstream result;
            size_t lastPos = 0;
            auto searchStart = content.cbegin();
            int currentLine = baseLineNumber;

            // Helper to count lines in a substring
            auto countLines = [](const std::string& str, size_t start, size_t end) -> int {
                if (start >= end)
                    return 0;
                return std::count(str.begin() + start, str.begin() + end, '\n');
                };

            // Helper to normalize paths
            auto normalizePath = [](const std::string& basePath, const std::string& relativePath) {
                if (relativePath.empty())
                    return std::string();

                std::string base = basePath;
                std::string rel = relativePath;

                // Remove filename from base path
                size_t lastSlash = base.find_last_of('/');
                if (lastSlash != std::string::npos)
                    base = base.substr(0, lastSlash + 1);
                else
                    base = "";

                // Resolve ../ in relative path
                while (rel.find("../") == 0) {
                    // Remove ../ from the beginning
                    rel = rel.substr(3);

                    // Go up one directory in base path
                    if (!base.empty()) {
                        size_t slash = base.find_last_of('/', base.length() - 2);
                        if (slash != std::string::npos)
                            base = base.substr(0, slash + 1);
                        else
                            base = "";
                    }
                }

                return base + rel;
                };

            // Process all #include directives
            while (std::regex_search(searchStart, content.cend(), match, includePattern)) {
                size_t matchPos = match.position() + (searchStart - content.cbegin());
                int linesBeforeMatch = countLines(content, lastPos, matchPos);

                // Append content before the match
                result << content.substr(lastPos, matchPos - lastPos);

                std::string includedFile = match[1].str();
                std::string resolvedPath = normalizePath(filePath, includedFile);
                int includeLineNumber = currentLine + linesBeforeMatch;

                if (!resolvedPath.empty()) {
                    try {
                        // Add line directive before including file
                        result << "#line 1 \"" << includedFile << "\"\n";

                        // Recursively resolve included file
                        std::string includedContent = resolveImpl(resolvedPath, 1);
                        result << includedContent;

                        // Add line directive after including file
                        result << "#line " << (includeLineNumber + 1) << "\n";

                    } catch (const std::exception& e) {
                        // Inclusion failed, output error comment
                        result << match.str();
                        result << "\n// Error: Failed to include '" << includedFile << "': ";
                        result << e.what() << "\n";
                        result << "#line " << (includeLineNumber + 1) << "\n";
                    }
                } else {
                    // Could not resolve path
                    result << match.str();
                    result << "\n#line " << (includeLineNumber + 1) << "\n";
                }

                lastPos = matchPos + match.length();
                searchStart = content.cbegin() + lastPos;
                currentLine = includeLineNumber + 1;
            }

            // Append remaining content
            result << content.substr(lastPos);
            m_processingFiles.erase(filePath);

            return result.str();

        } catch (...) {
            m_processingFiles.erase(filePath);
        }
        return "// Error: Failed to include file: " + filePath + "\n";
    }

private:
    // Set of files currently being processed to detect circular dependencies
    std::set<std::string> m_processingFiles = {};
};

/**
 * @brief Resolves #include directives in the given shader file.
 * @param name The path of the shader file to resolve.
 * @return The resolved shader string with all includes processed.
 */
inline std::string getResolved(const std::string& name) {
    IncludeResolver resolver;
    return resolver.resolve(name);
}

}
