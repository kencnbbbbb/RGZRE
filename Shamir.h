#ifndef SHAMIR_H
#define SHAMIR_H

#include <string>

namespace Shamir {
std::string encrypt(const std::string& text, bool showSteps);
std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok);
void printKeys();
}

#endif
