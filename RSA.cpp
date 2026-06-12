#include "RSA.h"

#include "MathUtils.h"

#include <array>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

namespace RSA {
const unsigned int P = 61U;
const unsigned int Q = 53U;
const unsigned int N = P * Q;
const unsigned int PHI = (P - 1U) * (Q - 1U);
const unsigned int E = 17U;
const unsigned int D = 2753U;

void printKeys() {
    std::cout << "RSA: p=" << P << ", q=" << Q << ", n=" << N
              << ", phi=" << PHI << ", e=" << E << ", d=" << D << "\n";
}

extern "C" const char* cipherName() {
    return "Шифр RSA";
}

extern "C" const char* cipherEncryptedFile() {
    return "rsa_encrypted.txt";
}

extern "C" const char* cipherDecryptedFile() {
    return "rsa_decrypted.txt";
}

extern "C" const char* cipherKeyHint() {
    return "n e d, например: 3233 17 2753";
}

extern "C" const char* cipherGenerateKey() {
    static std::string generatedKey;
    static std::mt19937 generator(std::random_device{}());

    // Учебный генератор: выбирает маленькие простые числа, чтобы расчеты
    // помещались в unsigned int и были понятны при демонстрации.
    const std::array<unsigned int, 8> primes = {47U, 53U, 59U, 61U, 67U, 71U, 73U, 79U};
    const std::array<unsigned int, 6> publicExponents = {5U, 7U, 11U, 13U, 17U, 19U};
    std::uniform_int_distribution<unsigned int> primeIndex(0U, static_cast<unsigned int>(primes.size() - 1U));
    std::uniform_int_distribution<unsigned int> expIndex(0U, static_cast<unsigned int>(publicExponents.size() - 1U));

    unsigned int p = primes[primeIndex(generator)];
    unsigned int q = primes[primeIndex(generator)];
    while (q == p) {
        q = primes[primeIndex(generator)];
    }
    unsigned int n = p * q;
    unsigned int phi = (p - 1U) * (q - 1U);
    unsigned int e = publicExponents[expIndex(generator)];
    int d = MathUtils::modInverse(static_cast<int>(e), static_cast<int>(phi));
    while (d < 0) {
        e = publicExponents[expIndex(generator)];
        d = MathUtils::modInverse(static_cast<int>(e), static_cast<int>(phi));
    }

    std::ostringstream stream;
    stream << n << ' ' << e << ' ' << d;
    generatedKey = stream.str();
    return generatedKey.c_str();
}

bool parseThreeKeyValues(const std::string& key, unsigned int& first, unsigned int& second, unsigned int& third) {
    std::stringstream stream(key);
    stream >> first >> second >> third;
    return !stream.fail();
}

extern "C" int cipherEncrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    unsigned int n = 0U;
    unsigned int e = 0U;
    unsigned int d = 0U;
    if (!parseThreeKeyValues(key, n, e, d) || n <= 255U) {
        return 1;
    }
    std::ostringstream result;
    for (unsigned int i = 0U; i < input.size(); ++i) {
        unsigned int m = static_cast<unsigned char>(input[i]);
        unsigned int c = MathUtils::modPow(m, e, n);
        if (i > 0U) {
            result << ' ';
        }
        result << c;
        if (showSteps) {
            std::cout << "Байт " << m << ": c = " << m << "^" << e << " mod " << n
                      << " = " << c << "\n";
        }
    }
    output = result.str();
    return 0;
}

extern "C" int cipherDecrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    unsigned int n = 0U;
    unsigned int e = 0U;
    unsigned int d = 0U;
    if (!parseThreeKeyValues(key, n, e, d) || n <= 255U) {
        return 1;
    }
    std::vector<unsigned int> values;
    if (!MathUtils::parseUnsignedList(input, values)) {
        return 1;
    }
    output.clear();
    for (unsigned int c : values) {
        unsigned int m = MathUtils::modPow(c, d, n);
        if (m > 255U) {
            return 1;
        }
        output.push_back(static_cast<char>(m));
        if (showSteps) {
            std::cout << "Число " << c << ": m = " << c << "^" << d << " mod " << n
                      << " = " << m << "\n";
        }
    }
    return 0;
}

extern "C" void cipherPrintInfo() {
    RSA::printKeys();
    std::cout << "Плагин RSA принимает ключ n e d и загружается динамически.\n";
    std::cout << "Режим учебный: генератор использует небольшие простые числа, это не промышленная криптография.\n";
}

std::string encrypt(const std::string& text, bool showSteps) {
    std::ostringstream result;
    if (showSteps) {
        printKeys();
    }
    for (unsigned int i = 0U; i < text.size(); ++i) {
        unsigned int m = static_cast<unsigned char>(text[i]);
        unsigned int c = MathUtils::modPow(m, E, N);
        if (i > 0U) {
            result << ' ';
        }
        result << c;
        if (showSteps) {
            std::cout << "Байт " << m << ": c = " << m << "^" << E << " mod " << N
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
        unsigned int m = MathUtils::modPow(c, D, N);
        if (m > 255U) {
            ok = false;
            return "";
        }
        result.push_back(static_cast<char>(m));
        if (showSteps) {
            std::cout << "Число " << c << ": m = " << c << "^" << D << " mod " << N
                      << " = " << m << "\n";
        }
    }
    return result;
}

}
