#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

namespace FileUtils {
bool fileExists(const std::string& path);
bool ensureParentDirectory(const std::string& path);
bool writeTextFile(const std::string& path, const std::string& content);
bool readTextFile(const std::string& path, std::string& content);
bool writeBinaryFile(const std::string& path, const std::string& content);
bool readBinaryFile(const std::string& path, std::string& content);
}

#endif
