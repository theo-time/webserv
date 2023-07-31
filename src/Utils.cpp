
#include "Utils.hpp"
#include <sys/stat.h>
#include <sys/time.h>

bool fileExists(std::string path) {
    struct stat fileInfo;
    if (stat(path.c_str(), &fileInfo) != 0) {
        // Failed to retrieve file information
        return false;
    }

    return S_ISREG(fileInfo.st_mode);
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

std::vector<std::string> splitWithSep(std::string line, char sep)
{
	std::vector<std::string> res;
	std::istringstream s(line);

	while (std::getline(s, line, sep))
		if (!line.empty())
			res.push_back(line);
	
	return res;
}

std::string getFileExtension(std::string filename)
{
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    return extension;
}

std::string concatenateList(const std::list<std::string>& list) {
    std::ostringstream oss;
    for (std::list<std::string>::const_iterator it = list.begin(); it != list.end(); ++it) {
        oss << *it;
    }
    return oss.str();
}

std::vector<std::string> getFileList(std::string path) {
    DIR             *dir;
    struct dirent   *entry;
    std::vector<std::string> fileList;

    if ((dir = opendir(path.c_str())) == NULL)
        perror("opendir() error");
    else {
        while ((entry = readdir(dir)) != NULL)
            fileList.push_back(entry->d_name);
        closedir(dir);
    }
    return fileList;
}

std::string getRedirectionHTML(std::string url)
{
    std::stringstream ss;
    std::string str;

    ss << "<!DOCTYPE html>" << std::endl;
    ss << "<html>" << std::endl;
    ss << "<head>" << std::endl;
    ss << "<title>Redirection</title>" << std::endl;
    ss << "<meta http-equiv=\"refresh\" content=\"0; url=" << url << "\" />" << std::endl;
    ss << "</head>" << std::endl;
    ss << "<body>" << std::endl;
    ss << "<p>Redirection vers <a href=\"" << url << "\">" << url << "</a></p>" << std::endl;
    ss << "</body>" << std::endl;
    ss << "</html>" << std::endl;

    str = ss.str();
    return str;
}

void	ft_bzero(void *s, size_t n)
{
	size_t		i;

	if (s == NULL || n == 0)
		return ;
	i = 0;
	while (i < n)
	{
		((char *)s)[i] = 0;
		i++;
	}
}

std::string getContentInfo(Request & req, std::string str) {

    std::string contentPrefix = str;
    size_t startPos = req.getRequestString().find(contentPrefix);
    if (startPos == std::string::npos)
        return "";

    startPos += contentPrefix.length();
    size_t endPos = req.getRequestString().find_first_of("\r\n", startPos);
    if (endPos == std::string::npos)
        return "";

    std::string contentValue = req.getRequestString().substr(startPos, endPos - startPos);
    return contentValue;
}

bool hasDuplicateKeys(const std::string& longString) {
    std::map<std::string, int> keyCount;
    std::istringstream iss(longString);

    std::string line;
    while (std::getline(iss, line)) {
        size_t pos = line.find(": ");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            ++keyCount[key];
        }
    }

    for (std::map<std::string, int>::const_iterator it = keyCount.begin(); it != keyCount.end(); ++it) {
        if (it->second > 1) {
            return true; // Duplicate key found
        }
    }

    return false; // No duplicate keys found
}

// returns the filename + ext
std::string getFileFromPath(std::string const& path) {
    std::string output = path;
    size_t lastPartHead = output.find_last_of('/');
    return output.substr(lastPartHead);
}

std::string removeContentTypeHeader(const std::string& CGIResponse) {
    std::string result = CGIResponse;
    size_t content_type_pos = CGIResponse.find("Content-type:");
    if (content_type_pos != std::string::npos) {
            size_t end_of_line = CGIResponse.find("\r\n", content_type_pos);
            result.erase(content_type_pos, end_of_line - content_type_pos + 2); // +2 to remove '\r\n'
    }

    return result;
}

unsigned long ft_now()
{
	struct timeval	now;
	gettimeofday(&now, 0);
	return (now.tv_sec + now.tv_usec * 1e-6);
}
