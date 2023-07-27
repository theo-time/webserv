
#include "Utils.hpp"
#include <sys/stat.h>

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