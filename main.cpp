#include "FileUtils.h"
#include "PluginLoader.h"

#include <exception>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const unsigned int MAX_TEXT_INPUT_SIZE = 1024U * 1024U;
const unsigned int MAX_FILE_INPUT_SIZE = 100U * 1024U * 1024U;
const char* PASTE_START = "\033[200~";
const char* PASTE_END = "\033[201~";
const char* MULTILINE_END = "<<<END>>>";

std::string trim(const std::string& value) {
    unsigned int start = 0U;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    unsigned int end = static_cast<unsigned int>(value.size());
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1U]))) {
        --end;
    }
    return value.substr(start, end - start);
}

bool isQuitCommand(const std::string& command) {
    return command == "q" || command == "Q";
}

int parseMenuNumber(const std::string& command) {
    try {
        std::size_t pos = 0U;
        int number = std::stoi(command, &pos);
        return pos == command.size() ? number : -1;
    } catch (const std::exception&) {
        return -1;
    }
}

std::string readMenuCommand() {
    std::string line;
    std::getline(std::cin, line);
    return trim(line);
}

int readMenuChoice() {
    return parseMenuNumber(readMenuCommand());
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

void removeAll(std::string& text, const std::string& token) {
    std::size_t pos = 0U;
    while ((pos = text.find(token, pos)) != std::string::npos) {
        text.erase(pos, token.size());
    }
}

std::string cleanupPastedText(std::string text) {
    removeAll(text, PASTE_START);
    removeAll(text, PASTE_END);
    return text;
}

std::string readTextBlock(const std::string& prompt) {
    std::cout << prompt;
    std::string firstLine;
    std::getline(std::cin, firstLine);

    std::string result = firstLine;
    if (result.find(PASTE_START) != std::string::npos &&
        result.find(PASTE_END) == std::string::npos) {
        std::string line;
        while (std::getline(std::cin, line)) {
            result += "\n" + line;
            if (line.find(PASTE_END) != std::string::npos) {
                break;
            }
        }
        return cleanupPastedText(result);
    }

    if (trim(firstLine) == "<<<") {
        result.clear();
        std::string line;
        bool first = true;
        while (std::getline(std::cin, line)) {
            if (line == MULTILINE_END) {
                break;
            }
            if (!first) {
                result += "\n";
            }
            result += line;
            first = false;
        }
        return cleanupPastedText(result);
    }

    return cleanupPastedText(result);
}

std::string decodeFileUrlPath(const std::string& path) {
    std::string prefix = "file://";
    if (path.rfind(prefix, 0U) != 0U) {
        return path;
    }

    std::string encoded = path.substr(prefix.size());
    std::string decoded;
    for (unsigned int i = 0U; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2U < encoded.size()) {
            std::string hex = encoded.substr(i + 1U, 2U);
            int value = 0;
            std::stringstream stream;
            stream << std::hex << hex;
            stream >> value;
            if (!stream.fail()) {
                decoded.push_back(static_cast<char>(value));
                i += 2U;
                continue;
            }
        }
        decoded.push_back(encoded[i]);
    }
    return decoded;
}

std::string normalizeUserPath(std::string path) {
    path = cleanupPastedText(trim(path));
    if (path.size() >= 2U) {
        char first = path.front();
        char last = path.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            path = path.substr(1U, path.size() - 2U);
        }
    }
    path = decodeFileUrlPath(path);
    if (!path.empty() && path[0] == '~') {
        const char* home = std::getenv("HOME");
        if (home != nullptr && (path.size() == 1U || path[1] == '/')) {
            path = std::string(home) + path.substr(1U);
        }
    }

    std::string unescaped;
    for (unsigned int i = 0U; i < path.size(); ++i) {
        if (path[i] == '\\' && i + 1U < path.size()) {
            unescaped.push_back(path[i + 1U]);
            ++i;
        } else {
            unescaped.push_back(path[i]);
        }
    }
    return unescaped;
}

std::string readPath(const std::string& prompt) {
    return normalizeUserPath(readLine(prompt));
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
    std::string text = readTextBlock(
        "Введите текст для шифрования. Для многострочного ввода введите <<<, затем текст, затем <<<END>>>:\n"
    );
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
    std::cout << "Q. Вернуться назад\n";
    std::cout << "Выбор: ";
    std::string command = readMenuCommand();
    if (isQuitCommand(command)) {
        ok = false;
        return "";
    }
    int choice = parseMenuNumber(command);
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
        std::string text = readTextBlock(
            "Введите шифртекст. Для многострочного ввода введите <<<, затем текст, затем <<<END>>>:\n"
        );
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
    std::string inputPath = readPath("Путь к исходному файлу: ");
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
    std::string outputPath = readPath("Путь для зашифрованного файла: ");
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
    std::string inputPath = readPath("Путь к зашифрованному файлу: ");
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
    std::string outputPath = readPath("Путь для дешифрованного файла: ");
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
        std::cout << "Q. Вернуться назад\n";
        std::cout << "Выбор: ";
        std::string command = readMenuCommand();
        if (isQuitCommand(command)) {
            return;
        }
        int choice = parseMenuNumber(command);
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
            std::cout << "Q. Выход\n";
            std::cout << "Выбор: ";

            std::string command = readMenuCommand();
            if (isQuitCommand(command)) {
                std::cout << "Работа завершена.\n";
                break;
            }
            int choice = parseMenuNumber(command);
            if (choice >= 1 && choice <= static_cast<int>(plugins.size())) {
                processCipher(plugins[static_cast<unsigned int>(choice - 1)]);
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
