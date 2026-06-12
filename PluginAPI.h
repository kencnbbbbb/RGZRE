#ifndef PLUGIN_API_H
#define PLUGIN_API_H

#include <string>

typedef const char* (*CipherTextInfoFunction)();
typedef int (*CipherTransformFunction)(const std::string&, const std::string&, bool, std::string&);
typedef void (*CipherPrintFunction)();

#endif
