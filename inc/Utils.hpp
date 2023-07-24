#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <cctype>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <cstring>
#include <sys/wait.h>
#include <stdexcept>
#include <list>
#include <fstream>
#include <algorithm>

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <cstdlib>
#include <list>


# include "WebServ.hpp"
# include "Location.hpp"
# include "Response.hpp"


bool fileExists(std::string path);

bool fileIsReadable(std::string path);

bool fileIsWritable(std::string path);

bool fileIsExecutable(std::string path);

std::vector<std::string> splitWithSep(std::string line, char sep);

std::string getFileExtension(std::string filename);

std::string concatenateList(const std::list<std::string>& list);

std::vector<std::string> getFileList(std::string path);

std::string getRedirectionHTML(std::string url);

#endif