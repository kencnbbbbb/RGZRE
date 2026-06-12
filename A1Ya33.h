#ifndef A1YA33_H
#define A1YA33_H

#include <string>

namespace A1Ya33 {
std::string encrypt(const std::string& text, bool showSteps);
std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok);
void printKeys();
}

#endif
