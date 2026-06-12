#include "Shamir.h"

#include "MathUtils.h"

#include <array>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

namespace Shamir {
const unsigned int P = 257U;
const unsigned int CA = 17U;
const unsigned int DA = 241U;
const unsigned int CB = 5U;
const unsigned int DB = 205U;

void printKeys() {
    std::cout << "Шамир: p=" << P << ", cA=" << CA << ", dA=" << DA
              << ", cB=" << CB << ", dB=" << DB << "\n";
}

extern "C" const char* cipherName() {
    return "Шифр Шамира";
}

extern "C" const char* cipherEncryptedFile() {
    return "shamir_encrypted.txt";
}

extern "C" const char* cipherDecryptedFile() {
    return "shamir_decrypted.txt";
}

extern "C" const char* cipherKeyHint() {
    return "p cA dA cB dB, например: 257 17 241 5 205";
}

extern "C" const char* cipherGenerateKey() {
    static std::string generatedKey;
    static std::mt19937 generator(std::random_device{}());

    // Учебный генератор: строит пару взаимно обратных показателей для
    // участников A и B по модулю p - 1.
    const std::array<unsigned int, 4> primes = {257U, 263U, 269U, 271U};
    const std::array<unsigned int, 9> exponents = {3U, 5U, 7U, 11U, 13U, 17U, 19U, 23U, 29U};
    std::uniform_int_distribution<unsigned int> primeIndex(0U, static_cast<unsigned int>(primes.size() - 1U));
    std::uniform_int_distribution<unsigned int> expIndex(0U, static_cast<unsigned int>(exponents.size() - 1U));

    unsigned int p = primes[primeIndex(generator)];
    unsigned int phi = p - 1U;
    unsigned int ca = exponents[expIndex(generator)];
    int da = MathUtils::modInverse(static_cast<int>(ca), static_cast<int>(phi));
    while (da < 0) {
        ca = exponents[expIndex(generator)];
        da = MathUtils::modInverse(static_cast<int>(ca), static_cast<int>(phi));
    }

    unsigned int cb = exponents[expIndex(generator)];
    int db = MathUtils::modInverse(static_cast<int>(cb), static_cast<int>(phi));
    while (db < 0) {
        cb = exponents[expIndex(generator)];
        db = MathUtils::modInverse(static_cast<int>(cb), static_cast<int>(phi));
    }

    std::ostringstream stream;
    stream << p << ' ' << ca << ' ' << da << ' ' << cb << ' ' << db;
    generatedKey = stream.str();
    return generatedKey.c_str();
}

bool parseShamirKey(const std::string& key, unsigned int& p, unsigned int& ca,
                    unsigned int& da, unsigned int& cb, unsigned int& db) {
    std::stringstream stream(key);
    stream >> p >> ca >> da >> cb >> db;
    return !stream.fail() && p > 255U && MathUtils::isPrime(static_cast<int>(p)) &&
           ((ca * da) % (p - 1U) == 1U) && ((cb * db) % (p - 1U) == 1U);
}

extern "C" int cipherEncrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    unsigned int p = 0U;
    unsigned int ca = 0U;
    unsigned int da = 0U;
    unsigned int cb = 0U;
    unsigned int db = 0U;
    if (!parseShamirKey(key, p, ca, da, cb, db)) {
        return 1;
    }
    std::ostringstream result;
    for (unsigned int i = 0U; i < input.size(); ++i) {
        unsigned int m = static_cast<unsigned char>(input[i]);
        unsigned int x1 = MathUtils::modPow(m, ca, p);
        unsigned int x2 = MathUtils::modPow(x1, cb, p);
        unsigned int x3 = MathUtils::modPow(x2, da, p);
        if (i > 0U) {
            result << ' ';
        }
        result << x3;
        if (showSteps) {
            std::cout << "m=" << m << ": x1=" << x1 << ", x2=" << x2 << ", c=" << x3 << "\n";
        }
    }
    output = result.str();
    return 0;
}

extern "C" int cipherDecrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    unsigned int p = 0U;
    unsigned int ca = 0U;
    unsigned int da = 0U;
    unsigned int cb = 0U;
    unsigned int db = 0U;
    if (!parseShamirKey(key, p, ca, da, cb, db)) {
        return 1;
    }
    std::vector<unsigned int> values;
    if (!MathUtils::parseUnsignedList(input, values)) {
        return 1;
    }
    output.clear();
    for (unsigned int c : values) {
        unsigned int m = MathUtils::modPow(c, db, p);
        if (m > 255U) {
            return 1;
        }
        output.push_back(static_cast<char>(m));
        if (showSteps) {
            std::cout << "c=" << c << ": m=c^dB mod p=" << m << "\n";
        }
    }
    return 0;
}

extern "C" void cipherPrintInfo() {
    Shamir::printKeys();
    std::cout << "Плагин Шамира принимает ключ p cA dA cB dB.\n";
    std::cout << "Режим учебный: ключи генерируются на малых числах для демонстрации алгоритма.\n";
}

std::string encrypt(const std::string& text, bool showSteps) {
    std::ostringstream result;
    if (showSteps) {
        printKeys();
    }
    for (unsigned int i = 0U; i < text.size(); ++i) {
        unsigned int m = static_cast<unsigned char>(text[i]);
        unsigned int x1 = MathUtils::modPow(m, CA, P);
        unsigned int x2 = MathUtils::modPow(x1, CB, P);
        unsigned int x3 = MathUtils::modPow(x2, DA, P);
        if (i > 0U) {
            result << ' ';
        }
        result << x3;
        if (showSteps) {
            std::cout << "m=" << m << ": x1=m^cA=" << x1
                      << ", x2=x1^cB=" << x2
                      << ", шифртекст=x2^dA=" << x3 << "\n";
        }
    }
    return result.str();
}

std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok) {
    std::vector<unsigned int> values;
    ok = MathUtils::parseUnsignedList(cipherText, values);
    if (!ok) {
        return "";
    }
    if (showSteps) {
        printKeys();
    }
    std::string result;
    for (unsigned int c : values) {
        unsigned int m = MathUtils::modPow(c, DB, P);
        if (m > 255U) {
            ok = false;
            return "";
        }
        result.push_back(static_cast<char>(m));
        if (showSteps) {
            std::cout << "c=" << c << ": m = c^dB mod p = " << m << "\n";
        }
    }
    return result;
}

}
