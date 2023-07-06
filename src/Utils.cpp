
#include "Utils.hpp"

bool fileExists(std::string path) {
    return (access(path.c_str(), F_OK) != -1);
}

bool fileIsReadable(std::string path) {
    return (access(path.c_str(), R_OK) != -1);
}

bool fileIsWritable(std::string path) {
    return (access(path.c_str(), W_OK) != -1);
}

bool fileIsExecutable(std::string path) {
    return (access(path.c_str(), X_OK) != -1);
}
