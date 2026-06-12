#include "A1Ya33.h"

#include "MathUtils.h"

#include <array>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace A1Ya33 {
const std::vector<std::string> LOWER = {
    "а", "б", "в", "г", "д", "е", "ё", "ж", "з", "и", "й",
    "к", "л", "м", "н", "о", "п", "р", "с", "т", "у", "ф",
    "х", "ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э", "ю", "я"
};
const std::vector<std::string> UPPER = {
    "А", "Б", "В", "Г", "Д", "Е", "Ё", "Ж", "З", "И", "Й",
    "К", "Л", "М", "Н", "О", "П", "Р", "С", "Т", "У", "Ф",
    "Х", "Ц", "Ч", "Ш", "Щ", "Ъ", "Ы", "Ь", "Э", "Ю", "Я"
};
const int A = 5;
const int B = 8;
const int A_INVERSE = 20;
const int MOD = 33;

std::string nextUtf8(const std::string& text, unsigned int& pos) {
    unsigned char lead = static_cast<unsigned char>(text[pos]);
    unsigned int length = 1U;
    if ((lead & 0xE0U) == 0xC0U) length = 2U;
    else if ((lead & 0xF0U) == 0xE0U) length = 3U;
    else if ((lead & 0xF8U) == 0xF0U) length = 4U;
    std::string token = text.substr(pos, length);
    pos += length;
    return token;
}

int indexOf(const std::vector<std::string>& alphabet, const std::string& token);
int positiveMod(int value);

extern "C" const char* cipherName() {
    return "Шифр А1Я33";
}

extern "C" const char* cipherEncryptedFile() {
    return "a1ya33_encrypted.txt";
}

extern "C" const char* cipherDecryptedFile() {
    return "a1ya33_decrypted.txt";
}

extern "C" const char* cipherKeyHint() {
    return "a b inverseA, например: 5 8 20";
}

extern "C" const char* cipherGenerateKey() {
    static std::string generatedKey;
    static std::mt19937 generator(std::random_device{}());

    // Учебный генератор: a выбирается взаимно простым с 33,
    // чтобы существовал обратный элемент для дешифрования.
    const std::array<int, 20> multipliers = {
        2, 4, 5, 7, 8, 10, 13, 14, 16, 17,
        19, 20, 23, 25, 26, 28, 29, 31, 32, 1
    };
    std::uniform_int_distribution<unsigned int> aIndex(0U, static_cast<unsigned int>(multipliers.size() - 1U));
    std::uniform_int_distribution<int> bValue(0, MOD - 1);

    int a = multipliers[aIndex(generator)];
    int b = bValue(generator);
    int inverse = MathUtils::modInverse(a, MOD);
    std::ostringstream stream;
    stream << a << ' ' << b << ' ' << inverse;
    generatedKey = stream.str();
    return generatedKey.c_str();
}

bool parseA1Key(const std::string& key, int& a, int& b, int& inverse) {
    std::stringstream stream(key);
    stream >> a >> b >> inverse;
    return !stream.fail() && MathUtils::gcd(a, MOD) == 1 && positiveMod(a * inverse) == 1;
}

std::string transformWithKey(const std::string& text, bool decryptMode, bool showSteps,
                             int a, int b, int inverse) {
    std::string result;
    unsigned int pos = 0U;
    while (pos < text.size()) {
        std::string token = nextUtf8(text, pos);
        bool upper = false;
        int index = indexOf(LOWER, token);
        if (index < 0) {
            index = indexOf(UPPER, token);
            upper = index >= 0;
        }
        if (index < 0) {
            result += token;
            if (showSteps) {
                std::cout << token << " оставлено без изменений\n";
            }
            continue;
        }
        int changed = decryptMode
            ? positiveMod(inverse * (index - b))
            : positiveMod(a * index + b);
        result += upper ? UPPER[static_cast<unsigned int>(changed)]
                        : LOWER[static_cast<unsigned int>(changed)];
        if (showSteps) {
            std::cout << token << ": " << (index + 1) << " -> " << (changed + 1) << "\n";
        }
    }
    return result;
}

extern "C" int cipherEncrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    int a = 0;
    int b = 0;
    int inverse = 0;
    if (!parseA1Key(key, a, b, inverse)) {
        return 1;
    }
    output = transformWithKey(input, false, showSteps, a, b, inverse);
    return 0;
}

extern "C" int cipherDecrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    int a = 0;
    int b = 0;
    int inverse = 0;
    if (!parseA1Key(key, a, b, inverse)) {
        return 1;
    }
    output = transformWithKey(input, true, showSteps, a, b, inverse);
    return 0;
}

extern "C" void cipherPrintInfo() {
    A1Ya33::printKeys();
    std::cout << "Плагин А1Я33 принимает ключ a b inverseA.\n";
    std::cout << "Режим учебный: генератор подбирает параметры аффинного преобразования русского алфавита.\n";
}

int indexOf(const std::vector<std::string>& alphabet, const std::string& token) {
    for (unsigned int i = 0U; i < alphabet.size(); ++i) {
        if (alphabet[i] == token) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int positiveMod(int value) {
    int result = value % MOD;
    if (result < 0) {
        result += MOD;
    }
    return result;
}

std::string transform(const std::string& text, bool decryptMode, bool showSteps) {
    std::string result;
    unsigned int pos = 0U;
    while (pos < text.size()) {
        std::string token = nextUtf8(text, pos);
        bool upper = false;
        int index = indexOf(LOWER, token);
        if (index < 0) {
            index = indexOf(UPPER, token);
            upper = index >= 0;
        }
        if (index < 0) {
            result += token;
            if (showSteps) {
                std::cout << token << " не входит в русский алфавит, оставлено без изменений\n";
            }
            continue;
        }
        int changed = decryptMode
            ? positiveMod(A_INVERSE * (index - B))
            : positiveMod(A * index + B);
        result += upper ? UPPER[static_cast<unsigned int>(changed)] : LOWER[static_cast<unsigned int>(changed)];
        if (showSteps) {
            std::cout << token << ": " << (index + 1) << " -> " << (changed + 1) << "\n";
        }
    }
    return result;
}

void printKeys() {
    std::cout << "А1Я33: русский алфавит 33 буквы, формула y=(5*x+8) mod 33\n";
}

std::string encrypt(const std::string& text, bool showSteps) {
    if (showSteps) {
        printKeys();
    }
    return transform(text, false, showSteps);
}

std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok) {
    ok = true;
    if (showSteps) {
        printKeys();
        std::cout << "Обратная формула x=20*(y-8) mod 33\n";
    }
    return transform(cipherText, true, showSteps);
}

}
