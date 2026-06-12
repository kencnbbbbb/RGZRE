#include "MasseyOmura.h"

#include "MathUtils.h"

#include <array>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

namespace MasseyOmura {
const unsigned int P = 257U;
const unsigned int E = 17U;
const unsigned int D = 241U;

void printKeys() {
    std::cout << "Месси-Омура: p=" << P << ", e=" << E << ", d=" << D
              << ", e*d mod (p-1)=1\n";
}

extern "C" const char* cipherName() {
    return "Шифр Месси - Омуры";
}

extern "C" const char* cipherEncryptedFile() {
    return "massey_omura_encrypted.txt";
}

extern "C" const char* cipherDecryptedFile() {
    return "massey_omura_decrypted.txt";
}

extern "C" const char* cipherKeyHint() {
    return "p e d, например: 257 17 241";
}

extern "C" const char* cipherGenerateKey() {
    static std::string generatedKey;
    static std::mt19937 generator(std::random_device{}());

    // Учебный генератор: p выбирается немного больше 255, чтобы один байт
    // шифровался как одно число без больших целочисленных типов.
    const std::array<unsigned int, 4> primes = {257U, 263U, 269U, 271U};
    const std::array<unsigned int, 9> exponents = {3U, 5U, 7U, 11U, 13U, 17U, 19U, 23U, 29U};
    std::uniform_int_distribution<unsigned int> primeIndex(0U, static_cast<unsigned int>(primes.size() - 1U));
    std::uniform_int_distribution<unsigned int> expIndex(0U, static_cast<unsigned int>(exponents.size() - 1U));

    unsigned int p = primes[primeIndex(generator)];
    unsigned int phi = p - 1U;
    unsigned int e = exponents[expIndex(generator)];
    int d = MathUtils::modInverse(static_cast<int>(e), static_cast<int>(phi));
    while (d < 0) {
        e = exponents[expIndex(generator)];
        d = MathUtils::modInverse(static_cast<int>(e), static_cast<int>(phi));
    }

    std::ostringstream stream;
    stream << p << ' ' << e << ' ' << d;
    generatedKey = stream.str();
    return generatedKey.c_str();
}

bool parseMasseyKey(const std::string& key, unsigned int& p, unsigned int& e, unsigned int& d) {
    std::stringstream stream(key);
    stream >> p >> e >> d;
    return !stream.fail() && p > 255U && MathUtils::isPrime(static_cast<int>(p)) &&
           ((e * d) % (p - 1U) == 1U);
}

extern "C" int cipherEncrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    unsigned int p = 0U;
    unsigned int e = 0U;
    unsigned int d = 0U;
    if (!parseMasseyKey(key, p, e, d)) {
        return 1;
    }
    std::ostringstream result;
    for (unsigned int i = 0U; i < input.size(); ++i) {
        unsigned int m = static_cast<unsigned char>(input[i]);
        unsigned int c = MathUtils::modPow(m, e, p);
        if (i > 0U) {
            result << ' ';
        }
        result << c;
        if (showSteps) {
            std::cout << "m=" << m << ": c=m^" << e << " mod " << p << "=" << c << "\n";
        }
    }
    output = result.str();
    return 0;
}

extern "C" int cipherDecrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    unsigned int p = 0U;
    unsigned int e = 0U;
    unsigned int d = 0U;
    if (!parseMasseyKey(key, p, e, d)) {
        return 1;
    }
    std::vector<unsigned int> values;
    if (!MathUtils::parseUnsignedList(input, values)) {
        return 1;
    }
    output.clear();
    for (unsigned int c : values) {
        unsigned int m = MathUtils::modPow(c, d, p);
        if (m > 255U) {
            return 1;
        }
        output.push_back(static_cast<char>(m));
        if (showSteps) {
            std::cout << "c=" << c << ": m=c^" << d << " mod " << p << "=" << m << "\n";
        }
    }
    return 0;
}

extern "C" void cipherPrintInfo() {
    MasseyOmura::printKeys();
    std::cout << "Плагин Месси-Омуры принимает ключ p e d.\n";
    std::cout << "Режим учебный: p и показатели маленькие, чтобы расчеты были демонстрационными.\n";
}

std::string encrypt(const std::string& text, bool showSteps) {
    std::ostringstream result;
    if (showSteps) {
        printKeys();
    }
    for (unsigned int i = 0U; i < text.size(); ++i) {
        unsigned int m = static_cast<unsigned char>(text[i]);
        unsigned int c = MathUtils::modPow(m, E, P);
        if (i > 0U) {
            result << ' ';
        }
        result << c;
        if (showSteps) {
            std::cout << "m=" << m << ": c = " << m << "^" << E << " mod " << P
                      << " = " << c << "\n";
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
        unsigned int m = MathUtils::modPow(c, D, P);
        if (m > 255U) {
            ok = false;
            return "";
        }
        result.push_back(static_cast<char>(m));
        if (showSteps) {
            std::cout << "c=" << c << ": m = " << c << "^" << D << " mod " << P
                      << " = " << m << "\n";
        }
    }
    return result;
}

}
