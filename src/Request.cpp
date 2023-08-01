#include "Request.hpp"
#include "CGI.hpp"

#include <fstream>
#include <algorithm>

typedef std::map<std::string, VirtualServer*>   srvMap;

Request::Request(int clientSocket, int serverSocket) : clientSocket(clientSocket), serverSocket(serverSocket) {
    std::cout << "Request created" << std::endl;
    _response.setRequest(this);
    readingHeader = false;
    readingBody = false;
    chunkedBody = false;
    curChunkSize = -1;
    contentLength = -1;
    ready2send = false;
    lastActivityTime = ft_now();
    curRequestTime = 0;
    _config = NULL;
}


Request::~Request() {
    std::cout << "Request destroyed" << std::endl;
}


bool Request::parseRequest(){

    lastActivityTime = ft_now();

    std::string firstLine = requestString.substr(0, requestString.find("\r\n"));
    std::vector<std::string> tokens = splitWithSep(firstLine, ' ');
    if (tokens.size() != 3) {
        _response.sendError(400, ": a field from 1st request line is missing"); 
    }

    // std::cout << "REQUEST STRING" << requestString << std::endl;

	parseMethodToken(tokens[0]);
    if (!parseURI(tokens[1]) || !parseHTTPVersion(tokens[2]) || !parseHeaders())
        return false;

    getRequestConfig();
    if (_config == NULL)
    {    
        _response.sendError(404, ": virtual server configuration not found");
        return false;
    }

    if ((method == "GET" && !_config->isGetAllowed()) || (method == "POST" && !_config->isPostAllowed()))
    {    
        _response.sendError(405, ": Method not allowed");
        return false;
    }

    if (path == "/") {
        if (!_config->getRoot().empty()){
            path = path + _config->getRoot() + "/";
        }
        path = path + _config->getIndex();
    }

    path = "." + path;

    if(path.find(_config->getName()) != std::string::npos && _config->getType() == "std") {
        path.replace(path.find(_config->getName()), _config->getName().length(), _config->getRoot());
        if (!::fileExists(path) && method == "GET" && !_config->getRoot().empty())
        {
                path = path + "/" + _config->getIndex();
        }
    }

    std::cout << " ---------- HEADER VALUES ---------- " << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "Protocol: " << protocol << std::endl;
    std::cout << "Query: " << query << std::endl;
    return true;
}

void Request::parseMethodToken(const std::string& token)
{
	std::string methods[5] = {"GET", "HEAD", "POST", "DELETE", "PUT"};
	
	for (int i = 0; i < 5; ++i)
	{
		if (!token.compare(0, methods[i].size(), methods[i]) &&
				token.size() == methods[i].size())
		{
			method = methods[i];
			return ;
		}
	}
    _response.sendError(400, ": unknown method");
}

bool Request::parseURI(std::string token)
{
    if (token.size() > MAX_URI_LEN) {
        _response.sendError(414, ": URI too long");
        return false;
    }

	if (token[0] != '/') {
        _response.sendError(400, ": URI must begin with a /");
        return false;
    }
  
    query = "";

	size_t querryChar = token.find("?");
	if (querryChar != std::string::npos)
	{
		path = token.substr(0, querryChar);
		query = token.substr(querryChar + 1, token.size());
	}
	else
		path = token;
    return true;
}

bool Request::parseHTTPVersion(const std::string& token)
{
	if (token.size() < 7 || token.compare(0, 5, "HTTP/") || token.compare(6, 1, ".") || 
			!isdigit(static_cast<int>(token[5])) || !isdigit(static_cast<int>(token[7])))
    {
		_response.sendError(400, ": HTTP version not correct");
        return false;
    }
	protocol = token;
    return true;
}

bool Request::parseHeaders()
{
    if (requestHeaderString.size() > MAX_HEADER_LEN)
        _response.sendError(431, ": Header too long");

    std::string delimiter = "\r\n";
    size_t pos = 0;
    std::string tmpRequestString = requestString;

    size_t posSC = tmpRequestString.find(":");
    if (posSC == std::string::npos) {
		_response.sendError(400, ": No semicolon");
        return false;
    }
    if (hasDuplicateKeys(requestHeaderString))
    {
		_response.sendError(400, ": duplicated headers are not allowed");
        return false;
    }

    while ((pos = tmpRequestString.find(delimiter)) != std::string::npos) {
        std::string line = tmpRequestString.substr(0, pos);
        tmpRequestString.erase(0, pos + delimiter.length());
        size_t pos2 = line.find(": ");
        if(pos2 != std::string::npos) {
            std::string key = line.substr(0, pos2);
            std::string value = line.substr(pos2 + 2);
            headers[key] = value;
        }
    }
    //get contentLength if any
    if (headers.count("Content-Length") == 1)
    {
        char*               endPtr      = NULL;
        contentLength = static_cast<unsigned int>(strtoul(headers["Content-Length"].c_str(), &endPtr, 0));
    }
    
    return true;
}

bool Request::parseBody()
{
    if (chunkedBody)
        body = concatenateList(requestBodyList);
    else
        body = requestBodyString;
    
    if(body.size() > _config->getClientMaxBodySize()) 
    {
        _response.sendError(413, ": received more octets than max body size limit");
        return false;
    }
    return true;
}

void     Request::getRequestConfig()
{
    std::cout << "------- Config Routing ----------" << std::endl;

    std::cout << "request socket fd :" << getServerSocket() << std::endl;

    VirtualServer *server = NULL;

    // check if hostname is an alias
    if (Config::getHostsMap().count(getHeader("Host")))
        server = Config::getHostsMap()[getHeader("Host")];
    else // Search matching socket
    {
        std::vector <VirtualServer*>::iterator    it = Config::getVirtualServers().begin();
        while (it != Config::getVirtualServers().end())
        {
            if ((*it)->getFd() == getServerSocket())
            {
                server = *it; // first by default
                break;
            }
            it++;
        }
    }
    if (server == NULL)
    {
        std::cout << "No matching server found" << std::endl;
        std::cout << "|-------- End of Config Routing ---------|" << std::endl;
        setConfig(NULL);
        return;
    }
    
    /* Search matching location */
    setConfig(server->getLocations()[0]); // first by default

    if (server->getLocations().size() > 1)
    {
        std::vector <Location*>             locations   = server->getLocations();
        std::vector <Location*>::iterator   locIt       = locations.begin();
        std::vector <Location*>::iterator   locEnd      = locations.end();
        std::string                         path        = getPath();
        std::string                         extension   = getFileExtension(path);
        std::string                         location_path;

        // check if CGI
        bool isCGI = false;
        while(locIt != locEnd)
        {
            if ((*locIt)->getType() == "cgi")
            {
                // std::cout <<  (**locIt) << std::endl;
                if ((*locIt)->getExtension() == extension && (*locIt)->isAllowed(method))
                {
                    setConfig(*locIt);
                    isCGI = true;
                    break;
                }

            }
            locIt++;
        }
        
        // not CGI
        if (!isCGI)
        {
            locIt = locations.begin();
            while(locIt != locEnd)
            {
                location_path = (**locIt).getName();
                if (path.find(location_path) == 0)
                {
                    setConfig(*locIt);
                    break;
                }
                locIt++;
            }
        }
    }

    std::cout << "seleted location for path " << path << std::endl;
    std::cout <<  *_config << std::endl;

    std::cout << "|-------- End of Config Routing ---------|" << std::endl;
}


void Request::listDirectoryResponse()
{
    std::cout << "LIST DIRECTORY" << std::endl;

    std::vector<std::string> fileList = getFileList(path);

    std::stringstream ss;

    ss << "<!DOCTYPE html>" << std::endl;
    ss << "<html>" << std::endl;
    ss << "<head>" << std::endl;
    ss << "<title>Directory listing</title>" << std::endl;
    ss << "</head>" << std::endl;
    ss << "<body>" << std::endl;
    ss << "<h1>Directory listing</h1>" << std::endl;
    ss << "<ul>" << std::endl;
    while (!fileList.empty())
    {
        if (fileList.back() != ".." && fileList.back() != ".")
            ss << "<li><a href=\"" << path << fileList.back() << "\">" << fileList.back() << "</a></li>" << std::endl;
        fileList.pop_back();
    }
    ss << "</ul>" << std::endl;
    ss << "</body>" << std::endl;
    ss << "</html>" << std::endl;

    _response.setBody(ss.str());
    _response.setStatusCode("200");
    _response.setProtocol("HTTP/1.1");
    _response.setStatusText("OK");
    _response.setContentType("text/html");
    _response.send();
}

void Request::handleRequest() 
{
    lastActivityTime = ft_now();

    // Conf file variables
    std::string root = _config->getRoot(); 
    std::string index = _config->getIndex();

    if (!parseBody())
        return;

    std::cout << " ---------- CURRENT LOCATION VALUES ---------- " << std::endl;
    std::cout << "Location Name: " << _config->getName() << std::endl;
    std::cout << "Location Root: " << _config->getRoot() << std::endl;
    std::cout << "Location Index: " << _config->getIndex() << std::endl;

    if(path.find(_config->getName()) != std::string::npos && _config->getType() == "std") {
        path.replace(path.find(_config->getName()), _config->getName().length(), _config->getRoot());
        if (!::fileExists(path) && method == "GET")
            path = path + "/" + index;
    }

    // Check redirection
    if(_config->getType() == "http")
    {
        std::cout << "-------- Redirection -------" << std::endl;
        fileContent = getRedirectionHTML(_config->getPath());
        _response.setStatusCode("303");
        _response.setStatusText("Other");
        _response.setContentType("text/html");
        _response.setProtocol("HTTP/1.1");
        _response.setBody(fileContent);
        _response.send();
        return;
    }

    // If path is a directory without index file and autoIndex is on
    std::string myPath = path;
    DIR *dir = opendir(myPath.c_str());
    if (dir)
    {
        if(myPath.find("//") != std::string::npos)
            myPath.replace(myPath.find("//"), 2, "/");
        if(!::fileExists(myPath))
        {
            if(!_config->isAutoIndex()) {
                closedir(dir);
                return(_response.sendError(403, "Forbidden"));
            }
            closedir(dir);
            return(listDirectoryResponse());
        }
        closedir(dir);
    }

    std::cout << "Path before method routing :" << getPath() << std::endl;

    // Method routing
    if (getFileExtension(path) == "php" || getFileExtension(path) == "py") {
        if (_config->getType() == "cgi")
            this->routingCGI();
        else {
            _response.sendError(405, "Don't match config file");
            return;
        }
    }
    else if (method == "GET")
        this->routingGet();
    else if(method == "POST" || method == "PUT")
        this->routingPost();
    else if(method == "DELETE")
        this->routingDelete();
    else if (method == "HEAD")
    {
        std::cout << "HEAD" << std::endl;
        _response.sendError(405, "Method Not Allowed");
        return;
    }
    else {
        std::cout << "Unknown method" << std::endl;
        _response.sendError(405, "Not Implemented");
    }
}

void Request::routingGet() 
{
    if(!::fileExists(path))
        _response.sendError(404, "Not Found");
    else if(!::fileIsReadable(path))
        _response.sendError(403, "Forbidden");
    else
    {
        _response.setProtocol("HTTP/1.1");
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        _response.send(path);
    }
}

void Request::routingPost() 
{
    std::ifstream my_file(path.c_str());

    //std::cout << body << std::endl;

    if (body.size() == 0 && readingBody == false)
    {
        std::cout << "Problem with BODY" << path << std::endl;
        std::cout << "Method not allowed" << std::endl;
        _response.sendError(405, "Body size is 0");
        return;
    }
    
    if (!my_file.good())
    {
        std::cout << "File don't exist" << std::endl;
        _response.setStatusCode("201");
        _response.setStatusText("Created");
        _response.setContentType("text/plain");
    }
    else {
        std::cout << "File exist" << std::endl;
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        _response.setContentType("text/plain");
        _response.setBody(getBody());
    }

	std::fstream postFile;
	postFile.open(path.c_str(), std::ios::app);
	if (!postFile.is_open())
		std::cout << "failed to open file in post method" << std::endl;
	postFile << getBody();
    _response.setProtocol("HTTP/1.1");
    _response.send();
}

void Request::routingDelete() 
{
    std::cout << "DELETE" << std::endl;
    std::ifstream my_file(path.c_str());

    if (!my_file.good())
    {
        std::cout << "File not found" << std::endl;
        _response.sendError(404, "Not Found");
    }
    else 
    {
        std::cout << "Requesting ressource at path : " << path << std::endl;
        if(!remove(path.c_str()))
        {
            std::cout << "File deleted" << std::endl;
            _response.setStatusCode("200");
            _response.setStatusText("OK");
            _response.setProtocol("HTTP/1.1");
            _response.send();
        }
        else
        {
            std::cout << "Error deleting file" << std::endl;
            _response.sendError(500, ": Internal Server Error");
        }
    }
}

void Request::routingCGI()
{
    if (method == "GET") {
        if(!::fileExists(path)) {
            _response.sendError(404, "Not Found");
            return;
        }
        else if(!::fileIsReadable(path)) {
            _response.sendError(403, "Forbidden");
            return;
        }
    }
    if(getFileExtension(path) == "py" || getFileExtension(path) == "php")
    {
        executable_path = _config->getPath();
        if (getFileExtension(path) == "py") {
            script_path = path.substr(2);
        }
        else if (getFileExtension(path) == "php") {
            script_path = "";
        }
        CGI cgi(*this);
        if (!cgi.executeCGI(*this)) {
            _response.sendError(500, " : Error in CGI execution");
            return;
        }
        else {
            _response.setStatusCode("200");
            _response.setStatusText("OK");
            _response.setContentType("text/html");
            _response.setProtocol("HTTP/1.1");
            _response.setBody(cgi.getOutputCGI());
            _response.send();
            return;
        }
    }
}

void Request::buildResponse()
{
    std::string extension = _response.getExtension();
    if(extension == "")
        extension = getFileExtension(path);
    std::string filename = path.substr(path.find_last_of("/") + 1);
    std::string contentType = "text/plain";
    std::string contentDisposition = "inline";

    // Build response
    // _response.setStatusCode("200");
    // _response.setStatusText("OK"); 
    _response.setProtocol("HTTP/1.1");
    _response.setFilename(filename);
    _response.setExtension(extension);

    // if extension is not displayable, download it
    if(extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "gif" || extension == "bmp" || extension == "ico") {
        contentType = "image/" + extension;
        contentDisposition = "attachment";
    } else if (extension == "pdf") {
        contentType = "application/pdf";
        contentDisposition = "attachment";
    } else if (extension == "css") {
        contentType = "text/css";
    } else if (extension == "js") {
        contentType = "application/javascript";
    } else if (extension == "txt") {
        contentType = "text/plain";
    } else if (extension == "html") {
        contentType = "text/html";
    } else {
        contentType = "text/plain";
    }

    _response.setContentType(contentType);
    _response.setContentDisposition(contentDisposition);
    _response.buildHeader();
    _response.buildResponse();
}

// GETTERS

std::string Request::getPath() {
    return path;
}

std::string Request::getProtocol() {
    return protocol;
}

std::string Request::getBody() {
    return body;
}

std::string Request::getQuery() {
    return query;
}

std::string Request::getMethod() {
    return method;
}

int Request::getMethodCode() {
    return methodCode;
}

std::string Request::getRequestString() {
    return requestString;
}

int Request::getClientSocket() {
    return clientSocket;
}

int Request::getServerSocket() {
    return serverSocket;
}

std::string Request::getResponseString() {
    return _response.getResponse();
}

std::string Request::getHeader(std::string key) {
    if (headers.count(key) == 1)
        return headers[key];
    return "";
}

Location* Request::getConfig() {
    return _config;
}

// SETTERS 

void Request::setConfig(Location* config) {
    _config = config;
}

void Request::setFileContent(std::string &fc)
{
    std::cout << "Setting file content" << std::endl;
    // std::cout << fc << std::endl;
    fileContent = fc;
    _response.setBody(fileContent);
}

void Request::setResponseString(std::string &rs)
{
    responseString = rs;
}

void Request::setRequestString(std::string &rs)
{
    requestString = rs;
}

void Request::appendRequestString(std::string rs)
{
    requestString = requestString + rs;
}

void Request::clear(void)
{
    method.clear();
    path.clear();
    originalPath.clear();
    protocol.clear();
    requestString.clear();
    body.clear();
    responseString.clear();
    fileContent.clear();
    headers.clear();
    requestBodyString.clear();
    methodCode = 0;
    cgi_mode = false;
    _config = NULL;
    _response.clear();
    readingHeader = false;
    readingBody = false;
    chunkedBody = false;
    requestBodyList.clear();
    curChunkSize = -1;
    contentLength = -1;
    ready2send = false;
    lastActivityTime = ft_now();
    curRequestTime = 0;
    std::cout << "CLIENT SOCKET CLEARED: fd - " << getClientSocket() << ", curRequestTime=" << curRequestTime << std::endl;
}

int Request::getChunkedBodySize() {
    int totalBodySize = 0;
    std::list<std::string>::iterator it = requestBodyList.begin();
    while (it != requestBodyList.end())
    {
        totalBodySize = totalBodySize + static_cast<int>((*it).size());
        it++;
    }
    return totalBodySize;
}
