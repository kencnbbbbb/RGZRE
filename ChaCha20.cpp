#include "ChaCha20.h"

#include "MathUtils.h"

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

namespace ChaCha20 {
using Word = std::uint32_t;

std::array<Word, 8> KEY = {
    0x03020100U, 0x07060504U, 0x0B0A0908U, 0x0F0E0D0CU,
    0x13121110U, 0x17161514U, 0x1B1A1918U, 0x1F1E1D1CU
};
std::array<Word, 3> NONCE = {0x00000000U, 0x4A000000U, 0x00000000U};
Word COUNTER = 1U;

Word rotateLeft(Word value, int shift) {
    return (value << shift) | (value >> (32 - shift));
}

bool parseHexWord(const std::string& text, unsigned int start, Word& value) {
    if (start + 8U > text.size()) {
        return false;
    }
    std::stringstream stream;
    stream << std::hex << text.substr(start, 8U);
    stream >> value;
    return !stream.fail();
}

bool applyUserKey(const std::string& key) {
    if (key == "default") {
        KEY = {
            0x03020100U, 0x07060504U, 0x0B0A0908U, 0x0F0E0D0CU,
            0x13121110U, 0x17161514U, 0x1B1A1918U, 0x1F1E1D1CU
        };
        NONCE = {0x00000000U, 0x4A000000U, 0x00000000U};
        COUNTER = 1U;
        return true;
    }

    std::stringstream stream(key);
    std::string keyHex;
    std::string nonceHex;
    Word counter = 0U;
    stream >> keyHex >> nonceHex >> counter;
    if (stream.fail() || keyHex.size() != 64U || nonceHex.size() != 24U) {
        return false;
    }
    for (unsigned int i = 0U; i < 8U; ++i) {
        if (!parseHexWord(keyHex, i * 8U, KEY[i])) {
            return false;
        }
    }
    for (unsigned int i = 0U; i < 3U; ++i) {
        if (!parseHexWord(nonceHex, i * 8U, NONCE[i])) {
            return false;
        }
    }
    COUNTER = counter;
    return true;
}

std::string randomHex(unsigned int digits, std::mt19937& generator) {
    const char* alphabet = "0123456789ABCDEF";
    std::uniform_int_distribution<unsigned int> value(0U, 15U);
    std::string result;
    for (unsigned int i = 0U; i < digits; ++i) {
        result.push_back(alphabet[value(generator)]);
    }
    return result;
}

extern "C" const char* cipherName() {
    return "Шифр ChaCha20";
}

extern "C" const char* cipherEncryptedFile() {
    return "chacha20_encrypted.txt";
}

extern "C" const char* cipherDecryptedFile() {
    return "chacha20_decrypted.txt";
}

extern "C" const char* cipherKeyHint() {
    return "64 hex символа ключа, 24 hex символа nonce, counter; пример: default";
}

extern "C" const char* cipherGenerateKey() {
    static std::string generatedKey;
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<unsigned int> counter(1U, 1000U);

    // Учебный генератор: формирует 256-битный ключ, 96-битный nonce и
    // небольшой счетчик в текстовом формате, который принимает плагин.
    std::ostringstream stream;
    stream << randomHex(64U, generator) << ' ' << randomHex(24U, generator)
           << ' ' << counter(generator);
    generatedKey = stream.str();
    return generatedKey.c_str();
}

extern "C" int cipherEncrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    if (!applyUserKey(key)) {
        return 1;
    }
    output = ChaCha20::encrypt(input, showSteps);
    return 0;
}

extern "C" int cipherDecrypt(const std::string& input, const std::string& key, bool showSteps, std::string& output) {
    if (!applyUserKey(key)) {
        return 1;
    }
    bool ok = false;
    output = ChaCha20::decrypt(input, showSteps, ok);
    return ok ? 0 : 1;
}

extern "C" void cipherPrintInfo() {
    ChaCha20::printKeys();
    std::cout << "Плагин ChaCha20 реализует quarter round, key stream и повторный XOR.\n";
    std::cout << "Режим учебный: генератор выдает hex-ключ, nonce и счетчик для демонстрации.\n";
}

void quarterRound(Word& a, Word& b, Word& c, Word& d) {
    a += b; d ^= a; d = rotateLeft(d, 16);
    c += d; b ^= c; b = rotateLeft(b, 12);
    a += b; d ^= a; d = rotateLeft(d, 8);
    c += d; b ^= c; b = rotateLeft(b, 7);
}

std::array<Word, 16> initialState(Word counter) {
    return {
        0x61707865U, 0x3320646EU, 0x79622D32U, 0x6B206574U,
        KEY[0], KEY[1], KEY[2], KEY[3], KEY[4], KEY[5], KEY[6], KEY[7],
        counter, NONCE[0], NONCE[1], NONCE[2]
    };
}

std::string serialize(const std::array<Word, 16>& state) {
    std::string bytes;
    for (Word word : state) {
        bytes.push_back(static_cast<char>(word & 0xFFU));
        bytes.push_back(static_cast<char>((word >> 8U) & 0xFFU));
        bytes.push_back(static_cast<char>((word >> 16U) & 0xFFU));
        bytes.push_back(static_cast<char>((word >> 24U) & 0xFFU));
    }
    return bytes;
}

std::string block(Word counter, bool showSteps) {
    std::array<Word, 16> state = initialState(counter);
    std::array<Word, 16> working = state;
    for (int round = 0; round < 10; ++round) {
        quarterRound(working[0], working[4], working[8], working[12]);
        quarterRound(working[1], working[5], working[9], working[13]);
        quarterRound(working[2], working[6], working[10], working[14]);
        quarterRound(working[3], working[7], working[11], working[15]);
        quarterRound(working[0], working[5], working[10], working[15]);
        quarterRound(working[1], working[6], working[11], working[12]);
        quarterRound(working[2], working[7], working[8], working[13]);
        quarterRound(working[3], working[4], working[9], working[14]);
    }
    for (unsigned int i = 0U; i < working.size(); ++i) {
        working[i] += state[i];
    }
    if (showSteps) {
        std::cout << "Сгенерирован блок ключевого потока со счетчиком " << counter << "\n";
    }
    return serialize(working);
}

std::string applyStream(const std::string& input, bool showSteps) {
    std::string output;
    output.resize(input.size());
    Word counter = COUNTER;
    unsigned int offset = 0U;
    while (offset < input.size()) {
        std::string keyStream = block(counter, showSteps);
        for (unsigned int i = 0U; i < keyStream.size() && offset < input.size(); ++i, ++offset) {
            output[offset] = static_cast<char>(
                static_cast<unsigned char>(input[offset]) ^ static_cast<unsigned char>(keyStream[i])
            );
            if (showSteps && offset < 8U) {
                std::cout << "XOR байта " << offset << "\n";
            }
        }
        ++counter;
    }
    return output;
}

void printKeys() {
    std::cout << "ChaCha20: ключ 256 бит, nonce 96 бит, начальный счетчик " << COUNTER << "\n";
}

std::string encrypt(const std::string& text, bool showSteps) {
    if (showSteps) {
        printKeys();
    }
    return MathUtils::bytesToHex(applyStream(text, showSteps));
}

std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok) {
    std::string bytes;
    ok = MathUtils::hexToBytes(cipherText, bytes);
    if (!ok) {
        return "";
    }
    if (showSteps) {
        printKeys();
    }
    return applyStream(bytes, showSteps);
}

}
