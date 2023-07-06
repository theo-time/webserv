#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <unistd.h>

bool fileExists(std::string path);

bool fileIsReadable(std::string path);

bool fileIsWritable(std::string path);

bool fileIsExecutable(std::string path);

#endif