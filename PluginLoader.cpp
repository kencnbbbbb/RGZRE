#include "PluginLoader.h"

#include <dlfcn.h>
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace PluginLoader {

template <typename T>
T loadSymbol(void* handle, const char* name) {
    return reinterpret_cast<T>(dlsym(handle, name));
}

bool hasPluginExtension(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    return extension == ".so" || extension == ".dylib";
}

std::vector<CipherPlugin> loadPlugins(const std::string& directory) {
    std::vector<CipherPlugin> plugins;
    std::filesystem::path pluginDir(directory);
    if (!std::filesystem::exists(pluginDir)) {
        std::cout << "Папка плагинов не найдена: " << directory << "\n";
        return plugins;
    }

    std::vector<std::filesystem::path> paths;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(pluginDir)) {
        if (entry.is_regular_file() && hasPluginExtension(entry.path())) {
            paths.push_back(entry.path());
        }
    }
    std::sort(paths.begin(), paths.end());

    for (const std::filesystem::path& pluginPath : paths) {
        if (!hasPluginExtension(pluginPath)) {
            continue;
        }

        std::string path = pluginPath.string();
        void* handle = dlopen(path.c_str(), RTLD_NOW);
        if (handle == nullptr) {
            std::cout << "Не удалось загрузить библиотеку " << path << ": " << dlerror() << "\n";
            continue;
        }

        CipherTextInfoFunction name = loadSymbol<CipherTextInfoFunction>(handle, "cipherName");
        CipherTextInfoFunction encryptedFile = loadSymbol<CipherTextInfoFunction>(handle, "cipherEncryptedFile");
        CipherTextInfoFunction decryptedFile = loadSymbol<CipherTextInfoFunction>(handle, "cipherDecryptedFile");
        CipherTextInfoFunction keyHint = loadSymbol<CipherTextInfoFunction>(handle, "cipherKeyHint");
        CipherTextInfoFunction generateKey = loadSymbol<CipherTextInfoFunction>(handle, "cipherGenerateKey");
        CipherTransformFunction encrypt = loadSymbol<CipherTransformFunction>(handle, "cipherEncrypt");
        CipherTransformFunction decrypt = loadSymbol<CipherTransformFunction>(handle, "cipherDecrypt");
        CipherPrintFunction printInfo = loadSymbol<CipherPrintFunction>(handle, "cipherPrintInfo");

        if (name == nullptr || encryptedFile == nullptr || decryptedFile == nullptr ||
            keyHint == nullptr || generateKey == nullptr || encrypt == nullptr ||
            decrypt == nullptr || printInfo == nullptr) {
            std::cout << "Библиотека " << path << " не соответствует API плагина.\n";
            dlclose(handle);
            continue;
        }

        CipherPlugin plugin;
        plugin.handle = handle;
        plugin.path = path;
        plugin.name = name();
        plugin.encryptedFile = encryptedFile();
        plugin.decryptedFile = decryptedFile();
        plugin.keyHint = keyHint;
        plugin.generateKey = generateKey;
        plugin.encrypt = encrypt;
        plugin.decrypt = decrypt;
        plugin.printInfo = printInfo;
        plugins.push_back(plugin);
    }

    return plugins;
}

void unloadPlugins(std::vector<CipherPlugin>& plugins) {
    for (CipherPlugin& plugin : plugins) {
        if (plugin.handle != nullptr) {
            dlclose(plugin.handle);
            plugin.handle = nullptr;
        }
    }
    plugins.clear();
}

}
