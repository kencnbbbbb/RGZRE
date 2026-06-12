#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include "PluginAPI.h"

#include <string>
#include <vector>

struct CipherPlugin {
    void* handle;
    std::string path;
    std::string name;
    std::string encryptedFile;
    std::string decryptedFile;
    CipherTextInfoFunction keyHint;
    CipherTextInfoFunction generateKey;
    CipherTransformFunction encrypt;
    CipherTransformFunction decrypt;
    CipherPrintFunction printInfo;
};

namespace PluginLoader {
std::vector<CipherPlugin> loadPlugins(const std::string& directory);
void unloadPlugins(std::vector<CipherPlugin>& plugins);
}

#endif
