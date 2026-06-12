#include "Rabin.h"

#include "MathUtils.h"

#include <array>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

namespace Rabin {
const unsigned int P = 19U;
const unsigned int Q = 23U;
const unsigned int N = P * Q;

void printKeys() {
    std::cout << "Рабин: p=" << P << ", q=" << Q << ", n=" << N
              << ". Каждый байт делится на две тетрады 0..15.\n";
}

extern "C" const char* cipherName() {
    return "Шифр Рабина";
}

extern "C" const char* cipherEncryptedFile() {
    return "rabin_encrypted.txt";
}

extern "C" const char* cipherDecryptedFile() {
    return "rabin_decrypted.txt";
}

extern "C" const char* cipherKeyHint() {
    return "p q, например: 19 23";
}

extern "C" const char* cipherGenerateKey() {
    static std::string generatedKey;
    static std::mt19937 generator(std::random_device{}());

    // Учебный генератор: выбирает p и q так, чтобы n > 15*15.
    // Поэтому корень для тетрады 0..15 восстанавливается однозначным перебором.
    const std::array<unsigned int, 8> primes = {19U, 23U, 29U, 31U, 37U, 41U, 43U, 47U};
    std::uniform_int_distribution<unsigned int> index(0U, static_cast<unsigned int>(primes.size() - 1U));
    unsigned int p = primes[index(generator)];
    unsigned int q = primes[index(generator)];
    while (q == p) {
        q = primes[index(generator)];
    }

    std::ostringstream stream;
    stream << p << ' ' << q;
    generatedKey = stream.str();
    return generatedKey.c_str();
}

bool parseTwoKeyValues(const std::string& key, unsigned int& first, unsigned int& second) {
    std::stringstream stream(key);
    stream >> first >> second;
    return !stream.fail() && MathUtils::isPrime(static_cast<int>(first)) &&
           MathUtils::isPrime(static_cast<int>(second)) && first != second &&
           first * second > 225U;
}

extern "C" int cipherEncrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    unsigned int p = 0U;
    unsigned int q = 0U;
    if (!parseTwoKeyValues(key, p, q) || p < 17U || q < 17U) {
        return 1;
    }
    unsigned int n = p * q;
    std::ostringstream result;
    bool firstValue = true;
    for (unsigned char byte : input) {
        unsigned int parts[2] = {
            static_cast<unsigned int>((byte >> 4U) & 0x0FU),
            static_cast<unsigned int>(byte & 0x0FU)
        };
        for (unsigned int part : parts) {
            unsigned int c = MathUtils::modPow(part, 2U, n);
            if (!firstValue) {
                result << ' ';
            }
            result << c;
            firstValue = false;
            if (showSteps) {
                std::cout << "m=" << part << ": c=m^2 mod " << n << "=" << c << "\n";
            }
        }
    }
    output = result.str();
    return 0;
}

extern "C" int cipherDecrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    unsigned int p = 0U;
    unsigned int q = 0U;
    if (!parseTwoKeyValues(key, p, q) || p < 17U || q < 17U) {
        return 1;
    }
    unsigned int n = p * q;
    std::vector<unsigned int> values;
    if (!MathUtils::parseUnsignedList(input, values) || values.size() % 2U != 0U) {
        return 1;
    }
    output.clear();
    for (unsigned int i = 0U; i < values.size(); i += 2U) {
        unsigned int restored[2] = {0U, 0U};
        for (unsigned int j = 0U; j < 2U; ++j) {
            bool found = false;
            for (unsigned int candidate = 0U; candidate < 16U; ++candidate) {
                if (MathUtils::modPow(candidate, 2U, n) == values[i + j]) {
                    restored[j] = candidate;
                    found = true;
                    break;
                }
            }
            if (!found) {
                return 1;
            }
            if (showSteps) {
                std::cout << "c=" << values[i + j] << ": корень=" << restored[j] << "\n";
            }
        }
        output.push_back(static_cast<char>((restored[0] << 4U) | restored[1]));
    }
    return 0;
}

extern "C" void cipherPrintInfo() {
    Rabin::printKeys();
    std::cout << "Плагин Рабина принимает ключ p q и хранится отдельной динамической библиотекой.\n";
    std::cout << "Режим учебный: байт делится на тетрады, чтобы избежать больших чисел.\n";
}

std::string encrypt(const std::string& text, bool showSteps) {
    std::ostringstream result;
    if (showSteps) {
        printKeys();
    }
    bool first = true;
    for (unsigned char byte : text) {
        unsigned int parts[2] = {
            static_cast<unsigned int>((byte >> 4U) & 0x0FU),
            static_cast<unsigned int>(byte & 0x0FU)
        };
        for (unsigned int part : parts) {
            unsigned int c = MathUtils::modPow(part, 2U, N);
            if (!first) {
                result << ' ';
            }
            result << c;
            first = false;
            if (showSteps) {
                std::cout << "m=" << part << ": c = m^2 mod " << N << " = " << c << "\n";
            }
        }
    }
    return result.str();
}

std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok) {
    std::vector<unsigned int> values;
    ok = MathUtils::parseUnsignedList(cipherText, values);
    if (!ok || values.size() % 2U != 0U) {
        ok = false;
        return "";
    }
    if (showSteps) {
        printKeys();
    }
    std::string result;
    for (unsigned int i = 0U; i < values.size(); i += 2U) {
        unsigned int restored[2] = {0U, 0U};
        for (unsigned int j = 0U; j < 2U; ++j) {
            bool found = false;
            for (unsigned int candidate = 0U; candidate < 16U; ++candidate) {
                if (MathUtils::modPow(candidate, 2U, N) == values[i + j]) {
                    restored[j] = candidate;
                    found = true;
                    break;
                }
            }
            if (!found) {
                ok = false;
                return "";
            }
            if (showSteps) {
                std::cout << "c=" << values[i + j] << ": найден корень "
                          << restored[j] << " в диапазоне 0..15\n";
            }
        }
        unsigned char byte = static_cast<unsigned char>((restored[0] << 4U) | restored[1]);
        result.push_back(static_cast<char>(byte));
    }
    return result;
}

}
