#include "MathUtils.h"

#include <cctype>
#include <sstream>

namespace MathUtils {

int gcd(int a, int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        int remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

int extendedGcd(int a, int b, int& x, int& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    int x1 = 0;
    int y1 = 0;
    int result = extendedGcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return result;
}

int modInverse(int a, int mod) {
    int x = 0;
    int y = 0;
    int result = extendedGcd(a, mod, x, y);
    if (result != 1) {
        return -1;
    }
    int inverse = x % mod;
    if (inverse < 0) {
        inverse += mod;
    }
    return inverse;
}

bool areCoprime(int a, int b) {
    return gcd(a, b) == 1;
}

bool isPrime(int value) {
    if (value < 2) {
        return false;
    }
    for (int divider = 2; divider * divider <= value; ++divider) {
        if (value % divider == 0) {
            return false;
        }
    }
    return true;
}

unsigned int modPow(unsigned int base, unsigned int exponent, unsigned int mod) {
    if (mod == 0) {
        return 0;
    }
    unsigned int result = 1U;
    base %= mod;
    while (exponent > 0U) {
        if ((exponent & 1U) != 0U) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exponent >>= 1U;
    }
    return result;
}

std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream stream(text);
    std::string item;
    while (std::getline(stream, item, delimiter)) {
        if (!item.empty()) {
            parts.push_back(item);
        }
    }
    return parts;
}

bool parseUnsignedList(const std::string& text, std::vector<unsigned int>& values) {
    values.clear();
    std::stringstream stream(text);
    std::string token;
    while (stream >> token) {
        for (char ch : token) {
            if (!std::isdigit(static_cast<unsigned char>(ch))) {
                return false;
            }
        }
        std::stringstream converter(token);
        unsigned int value = 0U;
        converter >> value;
        if (converter.fail()) {
            return false;
        }
        values.push_back(value);
    }
    return !values.empty();
}

std::string bytesToHex(const std::string& data) {
    const char* digits = "0123456789ABCDEF";
    std::string result;
    for (unsigned char byte : data) {
        result.push_back(digits[(byte >> 4U) & 0x0FU]);
        result.push_back(digits[byte & 0x0FU]);
    }
    return result;
}

bool hexToBytes(const std::string& hex, std::string& data) {
    auto valueOf = [](char ch) -> int {
        if (ch >= '0' && ch <= '9') return ch - '0';
        if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
        return -1;
    };

    data.clear();
    std::string compact;
    for (char ch : hex) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            compact.push_back(ch);
        }
    }
    if (compact.size() % 2U != 0U) {
        return false;
    }
    for (unsigned int i = 0U; i < compact.size(); i += 2U) {
        int high = valueOf(compact[i]);
        int low = valueOf(compact[i + 1U]);
        if (high < 0 || low < 0) {
            return false;
        }
        data.push_back(static_cast<char>((high << 4) | low));
    }
    return true;
}

}
