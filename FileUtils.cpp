#include "FileUtils.h"

#include <filesystem>
#include <fstream>

namespace FileUtils {

bool fileExists(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::binary);
    return file.good();
}

bool ensureParentDirectory(const std::string& path) {
    std::filesystem::path filePath(path);
    std::filesystem::path parent = filePath.parent_path();
    if (parent.empty()) {
        return true;
    }
    std::error_code error;
    std::filesystem::create_directories(parent, error);
    return !error;
}

bool writeTextFile(const std::string& path, const std::string& content) {
    if (!ensureParentDirectory(path)) {
        return false;
    }
    std::ofstream file(path.c_str());
    if (!file.is_open()) {
        return false;
    }
    file << content;
    return true;
}

bool readTextFile(const std::string& path, std::string& content) {
    std::ifstream file(path.c_str());
    if (!file.is_open()) {
        return false;
    }
    content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return true;
}

bool writeBinaryFile(const std::string& path, const std::string& content) {
    if (!ensureParentDirectory(path)) {
        return false;
    }
    std::ofstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return true;
}

bool readBinaryFile(const std::string& path, std::string& content) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return true;
}

}
