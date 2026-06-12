#ifndef CHACHA20_H
#define CHACHA20_H

#include <string>

namespace ChaCha20 {
std::string encrypt(const std::string& text, bool showSteps);
std::string decrypt(const std::string& cipherText, bool showSteps, bool& ok);
void printKeys();
}

#endif
