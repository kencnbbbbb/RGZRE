#include "FileUtils.h"
#include "PluginLoader.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

const unsigned int MAX_TEXT_INPUT_SIZE = 1024U * 1024U;
const unsigned int MAX_FILE_INPUT_SIZE = 100U * 1024U * 1024U;

int readMenuChoice() {
    std::string line;
    std::getline(std::cin, line);
    try {
        return std::stoi(line);
    } catch (const std::exception&) {
        return -1;
    }
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

bool askSteps() {
    std::string answer = readLine("Показывать промежуточные этапы? (д/н): ");
    return answer == "д" || answer == "Д" || answer == "y" || answer == "Y";
}

std::string readKey(const CipherPlugin& plugin, std::string& sessionKey, bool allowGenerate) {
    std::cout << "Формат ключа: " << plugin.keyHint() << "\n";
    std::string key = readLine("Введите ключ или оставьте пустым для текущего/нового учебного ключа: ");
    if (key.empty()) {
        if (!sessionKey.empty()) {
            std::cout << "Использован текущий ключ: " << sessionKey << "\n";
            return sessionKey;
        }
        if (!allowGenerate) {
            std::cout << "Ошибка: для дешифрования нужен тот же ключ, который использовался при шифровании.\n";
            return "";
        }
        sessionKey = plugin.generateKey();
        std::cout << "Сгенерирован и использован учебный ключ: " << sessionKey << "\n";
        return sessionKey;
    }
    sessionKey = key;
    return key;
}

bool validateTextSize(const std::string& value) {
    if (value.empty()) {
        std::cout << "Ошибка: данные не должны быть пустыми.\n";
        return false;
    }
    if (value.size() > MAX_TEXT_INPUT_SIZE) {
        std::cout << "Ошибка: размер данных больше 1 МБ для учебной версии.\n";
        return false;
    }
    return true;
}

bool validateFileSize(const std::string& value) {
    if (value.empty()) {
        std::cout << "Ошибка: файл пуст.\n";
        return false;
    }
    if (value.size() > MAX_FILE_INPUT_SIZE) {
        std::cout << "Ошибка: размер файла больше 100 МБ для учебной версии.\n";
        return false;
    }
    return true;
}

std::string defaultEncryptedPath(const std::string& inputPath) {
    return inputPath + ".encrypted";
}

std::string defaultDecryptedPath(const std::string& inputPath) {
    std::filesystem::path path(inputPath);
    std::string name = path.filename().string();
    std::filesystem::path parent = path.parent_path();
    std::string outputName = "decrypted_" + name;
    if (parent.empty()) {
        return outputName;
    }
    return (parent / outputName).string();
}

void encryptText(const CipherPlugin& plugin, std::string& sessionKey) {
    std::string text = readLine("Введите текст для шифрования: ");
    if (!validateTextSize(text)) {
        return;
    }
    std::string key = readKey(plugin, sessionKey, true);
    bool showSteps = askSteps();
    std::string encrypted;
    if (plugin.encrypt(text, key, showSteps, encrypted) == 0) {
        std::cout << "Зашифрованный текст:\n" << encrypted << "\n";
        if (FileUtils::writeTextFile(plugin.encryptedFile, encrypted)) {
            std::cout << "Файл создан: " << plugin.encryptedFile << "\n";
        } else {
            std::cout << "Ошибка записи файла.\n";
        }
    } else {
        std::cout << "Ошибка шифрования. Проверьте ключ и данные.\n";
    }
}

std::string readCipherText(const CipherPlugin& plugin, bool& ok) {
    std::cout << "1. Считать из файла " << plugin.encryptedFile << "\n";
    std::cout << "2. Ввести шифртекст вручную\n";
    std::cout << "Выбор: ";
    int choice = readMenuChoice();
    ok = true;
    if (choice == 1) {
        std::string content;
        if (!FileUtils::readTextFile(plugin.encryptedFile, content)) {
            std::cout << "Ошибка: файл не найден или недоступен.\n";
            ok = false;
            return "";
        }
        if (!validateTextSize(content)) {
            ok = false;
            return "";
        }
        return content;
    }
    if (choice == 2) {
        std::string text = readLine("Введите шифртекст: ");
        ok = validateTextSize(text);
        return text;
    }
    std::cout << "Ошибка: неверный пункт меню.\n";
    ok = false;
    return "";
}

void decryptText(const CipherPlugin& plugin, std::string& sessionKey) {
    bool inputOk = false;
    std::string cipherText = readCipherText(plugin, inputOk);
    if (!inputOk) {
        return;
    }
    std::string key = readKey(plugin, sessionKey, false);
    if (key.empty()) {
        return;
    }
    bool showSteps = askSteps();
    std::string decrypted;
    if (plugin.decrypt(cipherText, key, showSteps, decrypted) == 0) {
        std::cout << "Дешифрованный текст:\n" << decrypted << "\n";
        if (FileUtils::writeTextFile(plugin.decryptedFile, decrypted)) {
            std::cout << "Файл создан: " << plugin.decryptedFile << "\n";
        } else {
            std::cout << "Ошибка записи файла.\n";
        }
    } else {
        std::cout << "Ошибка дешифрования. Проверьте ключ и формат шифртекста.\n";
    }
}

void encryptFile(const CipherPlugin& plugin, std::string& sessionKey) {
    std::string inputPath = readLine("Путь к исходному файлу: ");
    if (inputPath.empty()) {
        std::cout << "Ошибка: путь не должен быть пустым.\n";
        return;
    }
    std::string input;
    if (!FileUtils::readBinaryFile(inputPath, input)) {
        std::cout << "Ошибка: файл не найден или недоступен.\n";
        return;
    }
    if (!validateFileSize(input)) {
        return;
    }
    std::cout << "Если оставить путь пустым, будет создан файл: "
              << defaultEncryptedPath(inputPath) << "\n";
    std::string outputPath = readLine("Путь для зашифрованного файла: ");
    if (outputPath.empty()) {
        outputPath = defaultEncryptedPath(inputPath);
    }
    std::string key = readKey(plugin, sessionKey, true);
    bool showSteps = askSteps();
    std::string encrypted;
    if (plugin.encrypt(input, key, showSteps, encrypted) != 0) {
        std::cout << "Ошибка шифрования файла.\n";
        return;
    }
    if (FileUtils::writeBinaryFile(outputPath, encrypted)) {
        std::cout << "Результат записан: " << outputPath << "\n";
    } else {
        std::cout << "Ошибка записи результата.\n";
    }
}

void decryptFile(const CipherPlugin& plugin, std::string& sessionKey) {
    std::string inputPath = readLine("Путь к зашифрованному файлу: ");
    if (inputPath.empty()) {
        std::cout << "Ошибка: путь не должен быть пустым.\n";
        return;
    }
    std::string cipherText;
    if (!FileUtils::readBinaryFile(inputPath, cipherText)) {
        std::cout << "Ошибка: файл не найден или недоступен.\n";
        return;
    }
    if (!validateFileSize(cipherText)) {
        return;
    }
    std::cout << "Если оставить путь пустым, будет создан файл: "
              << defaultDecryptedPath(inputPath) << "\n";
    std::string outputPath = readLine("Путь для дешифрованного файла: ");
    if (outputPath.empty()) {
        outputPath = defaultDecryptedPath(inputPath);
    }
    std::string key = readKey(plugin, sessionKey, false);
    if (key.empty()) {
        return;
    }
    bool showSteps = askSteps();
    std::string decrypted;
    if (plugin.decrypt(cipherText, key, showSteps, decrypted) != 0) {
        std::cout << "Ошибка дешифрования файла.\n";
        return;
    }
    if (FileUtils::writeBinaryFile(outputPath, decrypted)) {
        std::cout << "Результат записан: " << outputPath << "\n";
    } else {
        std::cout << "Ошибка записи результата.\n";
    }
}

void processCipher(const CipherPlugin& plugin) {
    std::string sessionKey;
    while (true) {
        std::cout << "\n" << plugin.name << "\n";
        std::cout << "1. Зашифровать текст\n";
        std::cout << "2. Дешифровать текст\n";
        std::cout << "3. Зашифровать файл по пути\n";
        std::cout << "4. Дешифровать файл по пути\n";
        std::cout << "5. Генератор ключей\n";
        std::cout << "6. Показать этапы / параметры алгоритма\n";
        std::cout << "7. Вернуться назад\n";
        std::cout << "Выбор: ";
        int choice = readMenuChoice();
        if (choice == 1) {
            encryptText(plugin, sessionKey);
        } else if (choice == 2) {
            decryptText(plugin, sessionKey);
        } else if (choice == 3) {
            encryptFile(plugin, sessionKey);
        } else if (choice == 4) {
            decryptFile(plugin, sessionKey);
        } else if (choice == 5) {
            sessionKey = plugin.generateKey();
            std::cout << "Сгенерированный учебный ключ: " << sessionKey << "\n";
        } else if (choice == 6) {
            plugin.printInfo();
        } else if (choice == 7) {
            return;
        } else {
            std::cout << "Ошибка: неверный пункт меню.\n";
        }
    }
}

int main() {
    try {
        std::vector<CipherPlugin> plugins = PluginLoader::loadPlugins("plugins");
        if (plugins.empty()) {
            std::cout << "Нет подключенных библиотек шифрования. Соберите проект командой make.\n";
            return 1;
        }

        while (true) {
            std::cout << "\nГлавное меню\n";
            for (unsigned int i = 0U; i < plugins.size(); ++i) {
                std::cout << (i + 1U) << ". " << plugins[i].name << "\n";
            }
            std::cout << (plugins.size() + 1U) << ". Выход\n";
            std::cout << "Выбор: ";

            int choice = readMenuChoice();
            if (choice >= 1 && choice <= static_cast<int>(plugins.size())) {
                processCipher(plugins[static_cast<unsigned int>(choice - 1)]);
            } else if (choice == static_cast<int>(plugins.size() + 1U)) {
                std::cout << "Работа завершена.\n";
                break;
            } else {
                std::cout << "Ошибка: неверный пункт меню.\n";
            }
        }

        PluginLoader::unloadPlugins(plugins);
    } catch (const std::exception& error) {
        std::cout << "Непредвиденная ошибка: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
