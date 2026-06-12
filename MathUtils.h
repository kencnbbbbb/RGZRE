#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <string>
#include <vector>

namespace MathUtils {
int gcd(int a, int b);
int extendedGcd(int a, int b, int& x, int& y);
int modInverse(int a, int mod);
bool areCoprime(int a, int b);
bool isPrime(int value);
unsigned int modPow(unsigned int base, unsigned int exponent, unsigned int mod);
std::vector<std::string> split(const std::string& text, char delimiter);
bool parseUnsignedList(const std::string& text, std::vector<unsigned int>& values);
std::string bytesToHex(const std::string& data);
bool hexToBytes(const std::string& hex, std::string& data);
}

#endif
