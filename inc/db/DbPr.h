/**
 * @file DbPr.h
 * @brief Private utilities for the DB module.
 */

#pragma once

#include "db/DbCommon.h"
#include "db/DbPub.h"

namespace DbFileUtils {

/**
 * @brief Create a temporary file in the same directory as the target file.
 * @param filename The target filename.
 * @return The path to the created temporary file.
 */
std::string createTempFile(const std::string& filename);
/**
 * @brief Replace the target file with the temporary file.
 * @param trgFilename The target filename.
 * @param tmpFilename The temporary filename.
 * @return 0 on success, non-zero on failure.
 */
int replaceFile(const std::string& trgFilename, const std::string& tmpFilename);
/**
 * @brief Get the relative path from basePath to targetPath.
 * @param basePath The base path.
 * @param targetPath The target path.
 * @return The relative path from basePath to targetPath.
 */
std::string getRelativePath(const std::string& basePath, const std::string& targetPath);
/**
 * @brief Get the absolute path by combining basePath and relativePath.
 * @param basePath The base path.
 * @param relativePath The relative path.
 * @return The absolute path.
 */
std::string getAbsolutePath(const std::string& basePath, const std::string& relativePath);

} // namespace DbFileUtils
