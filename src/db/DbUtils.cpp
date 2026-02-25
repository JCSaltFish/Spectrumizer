/**
 * @file DbUtils.cpp
 * @brief Implementation of utility functions for the DB module.
 */

#include "db/DbPr.h"

#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

std::string DbFileUtils::createTempFile(const std::string& filename) {
    std::filesystem::path filePath(filename);
    std::filesystem::path tmpFilePath =
        filePath.parent_path() / (filePath.filename().string() + ".tmp");
    if (std::filesystem::exists(tmpFilePath))
        std::filesystem::remove(tmpFilePath);
#ifdef _WIN32
    // Add FILE_ATTRIBUTE_TEMPORARY to hint the OS to keep the file in memory if possible
    HANDLE hFile = CreateFileA
    (
        tmpFilePath.string().c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_TEMPORARY,
        NULL
    );
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
#endif
    return tmpFilePath.string();
}

int DbFileUtils::replaceFile(const std::string& trgFilename, const std::string& tmpFilename) {
#ifdef _WIN32
    // Try to use ReplaceFileA first, which is atomic and preserves file attributes
    int res = 0;
    res = ReplaceFileA(
        trgFilename.c_str(),
        tmpFilename.c_str(),
        nullptr,
        REPLACEFILE_IGNORE_MERGE_ERRORS | REPLACEFILE_IGNORE_ACL_ERRORS,
        nullptr,
        nullptr
    );
    if (res)
        return 0;

    // If ReplaceFileA fails, try to move the temp file to the target location
    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND) {
        if (MoveFileA(tmpFilename.c_str(), trgFilename.c_str()))
            return 0;
    }

    // As a last resort, delete the target file and rename the temp file
    std::remove(trgFilename.c_str());
    if (std::rename(tmpFilename.c_str(), trgFilename.c_str()) == 0)
        return 0;
#else
    if (std::rename(tmpFilename.c_str(), trgFilename.c_str()) == 0)
        return 0;
#endif

    // If all methods fail, remove the temp file to avoid littering
    if (std::filesystem::exists(tmpFilename))
        std::filesystem::remove(tmpFilename);

    return 1;
}

std::string DbFileUtils::getRelativePath(
    const std::string& basePath,
    const std::string& targetPath
) {
    try {
        std::filesystem::path base = std::filesystem::absolute(basePath).lexically_normal();
        std::filesystem::path target = std::filesystem::absolute(targetPath).lexically_normal();

#ifdef _WIN32
        // On Windows, ensure both paths are on the same drive
        auto hasSameRootPath =
            [](const std::filesystem::path& path1, const std::filesystem::path& path2) {
            try {
                std::filesystem::path root1 = path1.root_path();
                std::filesystem::path root2 = path2.root_path();

                if (root1.empty() || root2.empty())
                    return true; // No drive letter, treat as same root

                std::string root1_str = root1.string();
                std::string root2_str = root2.string();
                // Case-insensitive comparison
                auto toLower = [](std::string str) {
                    std::transform(
                        str.begin(),
                        str.end(),
                        str.begin(),
                        [](unsigned char c) { return std::tolower(c); }
                    );
                    return str;
                    };
                return toLower(root1_str) == toLower(root2_str);
            } catch (...) { return false; } // On error, assume different roots
            };
        if (!hasSameRootPath(base, target))
            return target.string(); // Different drives, return absolute path
#endif

        // Use the directory of basePath if it's a file
        std::filesystem::path baseDir = base;
        if (std::filesystem::exists(base) && std::filesystem::is_regular_file(base))
            baseDir = base.parent_path();

        // Compute relative path
        std::filesystem::path relative = std::filesystem::relative(target, baseDir);

        if (relative.empty())
            return target.string(); // Could not compute relative path, return absolute

        return relative.string();
    } catch (...) {
        try { return std::filesystem::absolute(targetPath).string(); } // Fallback to absolute path
        catch (...) { return targetPath; } // Fallback to original path
    }
}

std::string DbFileUtils::getAbsolutePath(
    const std::string& basePath,
    const std::string& relativePath
) {
    try {
        std::filesystem::path base = std::filesystem::absolute(basePath).lexically_normal();
        std::filesystem::path relative(relativePath);

        if (relative.is_absolute())
            return relative.lexically_normal().string(); // Already absolute

        // Resolve relative path against base path
        std::filesystem::path baseDir = base;
        if (std::filesystem::exists(base) && std::filesystem::is_regular_file(base))
            baseDir = base.parent_path();

        // Combine and normalize
        std::filesystem::path absolute = (baseDir / relative).lexically_normal();
        return absolute.string();
    } catch (...) {
        // Fallback to current working directory
        try {
            return (std::filesystem::current_path() / relativePath).lexically_normal().string();
        } catch (...) { return relativePath; } // Fallback to original path
    }
}
