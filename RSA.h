#ifndef RSA_H
#define RSA_H

#include <string>

namespace RSA {
std::string encrypt(const std::string& text, bool showSteps);
std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok);
void printKeys();
}

#endif
