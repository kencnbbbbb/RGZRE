#ifndef RABIN_H
#define RABIN_H

#include <string>

namespace Rabin {
std::string encrypt(const std::string& text, bool showSteps);
std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok);
void printKeys();
}

#endif
