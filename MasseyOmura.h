#ifndef MASSEY_OMURA_H
#define MASSEY_OMURA_H

#include <string>

namespace MasseyOmura {
std::string encrypt(const std::string& text, bool showSteps);
std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok);
void printKeys();
}

#endif
