#include "Request.hpp"
#include "CGI.hpp"

#include <fstream>
#include <algorithm>


Request::Request(int clientSocket, int serverSocket) : clientSocket(clientSocket), serverSocket(serverSocket) {
    std::cout << "Request created" << std::endl;
    _response.setRequest(this);
    readingHeader = false;
    readingBody = false;
    chunkedBody = false;
    curChunkSize = -1;
    ready2send = false;
}


Request::~Request() {
    std::cout << "Request destroyed" << std::endl;
}


void Request::parseRequest(){
    std::string firstLine = requestString.substr(0, requestString.find("\r\n"));
    std::vector<std::string> tokens = splitWithSep(firstLine, ' ');
    if (tokens.size() != 3)
		std::cout << "400 : a field from request line is missing";

	parseMethodToken(tokens[0]);
    parseURI(tokens[1]);
	parseHTTPVersion(tokens[2]);
    parseHeaders();
    parseBody();


    std::cout << " - PARSING COMPLETED - " << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "Protocol: " << protocol << std::endl;
    std::cout << "Query: " << query << std::endl;
    std::cout << "Body: " << body << std::endl;
}



void Request::parseMethodToken(const std::string& token)
{
	std::string methods[4] = {"GET", "HEAD", "POST", "DELETE"};
	
	for (int i = 0; i < 4; ++i)
	{
		if (!token.compare(0, methods[i].size(), methods[i]) &&
				token.size() == methods[i].size())
		{
			method = methods[i];
			return ;
		}
	}

	std::cout << "400 : unknown method";
}

void Request::parseURI(std::string token)
{
	if (token[0] != '/')
		std::cout << "400 : URI must begin with a /" << std::endl;
	
    query = "";

	size_t querryChar = token.find("?");
	if (querryChar != std::string::npos)
	{
		path = token.substr(0, querryChar);
		query = token.substr(querryChar + 1, token.size());
	}
	else
		path = token;
    path = "." + path;
}

void Request::parseHTTPVersion(const std::string& token)
{
	if (token.size() < 7 || token.compare(0, 5, "HTTP/") || token.compare(6, 1, ".") || 
			!isdigit(static_cast<int>(token[5])) || !isdigit(static_cast<int>(token[7])))
		std::cout << "400 : HTTP version not correct";

	protocol = token;
}

void Request::parseHeaders()
{
    std::string delimiter = "\r\n";
    size_t pos = 0;
    int i = 0;
    while ((pos = requestString.find(delimiter)) != std::string::npos) {
        std::string token = requestString.substr(0, pos);
        requestString.erase(0, pos + delimiter.length());
        std::string delimiter2 = ": ";
        size_t pos2 = 0;
        std::string token2;
        int j = 0;
        while ((pos2 = token.find(delimiter2)) != std::string::npos) {
            token2 = token.substr(0, pos2);
            token.erase(0, pos2 + delimiter2.length());
            if (j == 0) {
                headers[token2] = token;
            }
            j++;
        }
        i++;
    }
    /*for (std::map<std::string, std::string>::iterator it=headers.begin(); it!=headers.end(); ++it)
        std::cout << it->first << " => " << it->second << '\n';*/
}

void Request::parseBody()
{
    if (chunkedBody)
        body = concatenateList(requestBodyList);
    else
        body = requestBodyString;
}

/*void Request::retrieveHeaderAndBody(const std::string& input) {
    std::string boundaryPrefix = "boundary=";
    size_t startPos = input.find(boundaryPrefix);
    if (startPos == std::string::npos) {
        // "boundary" identifier not found
        return;
    }

    startPos += boundaryPrefix.length();
    size_t endPos = input.find_first_of("\r\n", startPos);

    if (endPos == std::string::npos) {
        // Line break not found
        return;
    }

    std::string boundaryValue = input.substr(startPos, endPos - startPos);
    boundaryValue = "--" + boundaryValue;

    std::cout << "boundaryValue :" << boundaryValue << std::endl;

    // Find the start and end positions of the body
    std::string bodyStart = boundaryValue;
    size_t bodyStartPos = input.find(bodyStart);
    if (bodyStartPos == std::string::npos) {
        std::cout << "BODY START NOT FOUND" << std::endl;
        return;
    }

    bodyStartPos += bodyStart.length();
    size_t bodyEndPos = input.find(boundaryValue + "--", bodyStartPos);
    if (bodyEndPos == std::string::npos) {
        std::cout << "BODY END NOT FOUND" << std::endl;
        return;
    }

    // Extract the header and body
    header = input.substr(0, bodyStartPos - bodyStart.length());
    body = input.substr(bodyStartPos, bodyEndPos - bodyStartPos);
}*/



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
        ss << "<li><a href=\"" << originalPath << "/" << fileList.back() << "\">" << fileList.back() << "</a></li>" << std::endl;
        fileList.pop_back();
    }
    ss << "</ul>" << std::endl;
    ss << "</body>" << std::endl;
    ss << "</html>" << std::endl;

    // std::cout << ss.str() << std::endl;
    _response.setBody(ss.str());
    _response.setStatusCode("200");
    _response.setProtocol("HTTP/1.1");
    _response.setStatusText("OK");
    _response.setContentType("text/html");
    _response.buildHeader();
    _response.buildResponse();
    WebServ::addResponseToQueue(this);
}

void Request::handleRequest() 
{
    // Conf file variables
    std::string root = _config->getRoot(); 
    std::string index = _config->getIndex();

    if(_config->getName() != "_internal")
        path.replace(path.find(_config->getName()), _config->getName().length(), _config->getRoot());
    else 
        path = root + path;

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
        _response.buildHeader();
        _response.buildResponse();
        // std::cout << _response.getResponse() << std::endl;
        WebServ::addResponseToQueue(this);
        return;
    }

    // If path is a directory, and index file exists, add default index name to path
    if (opendir(path.c_str()))
    {
        std::string indexFile = path + "/" + index;
        std::cout << "Effective path: " << indexFile << std::endl;
        if(path.find("//") != std::string::npos)
            path.replace(path.find("//"), 2, "/");
        if(!::fileExists(indexFile))
        {
            if(!_config->isAutoIndex())
                return(_response.sendError(403, "Forbidden"));
            return(listDirectoryResponse());
        }
        else
            path = indexFile;
    }

    // Method routing
    if (getFileExtension(path) == "py" || getFileExtension(path) == "bla")
        this->routingCGI();
    else if (method == "GET")
        this->routingGet();
    else if(method == "POST")
        this->routingPost();
    else if(method == "DELETE")
        this->routingDelete();
    else if (method == "HEAD")
    {
        std::cout << "HEAD" << std::endl;
        _response.sendError(405, "Method Not Allowed");

        /* _response.setStatusCode("405");// TODO a enlever car juste pour tester
        _response.setStatusText("Method Not Allowed");
        _response.setContentType("text/html");
        _response.setProtocol("HTTP/1.1");
        WebServ::getRessource("./data/default/empty.html", *this); */
        return;
    }
    else {
        std::cout << "Unknown method" << std::endl;
        _response.setStatusCode("501");
        _response.setStatusText("Not Implemented");
        _response.setContentType("text/html");
        WebServ::getRessource("./data/default/501.html", *this);
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
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        std::cout << "Requesting ressource at path : " << path << std::endl;
        WebServ::getRessource(path, *this);
    }
}

void Request::routingPost() 
{
	std::fstream postFile;

	postFile.open(path.c_str(), std::ios::app);
	if (!postFile.is_open())
		std::cout << "failed to open file in post method" << std::endl;
	postFile << getBody();

    _response.setStatusCode("200");
    _response.setStatusText("OK");
    WebServ::getRessource(path, *this);

    //_response.sendError(405, "Method Not Allowed"); // TODO a enlever car juste pour tester
}

void Request::routingDelete() 
{
    std::cout << "DELETE" << std::endl;
    std::ifstream my_file(path.c_str());

    if (!my_file.good())
    {
        std::cout << "File not found" << std::endl;
        _response.setStatusCode("404");
        _response.setStatusText("Not Found");
        _response.setContentType("text/html");
        WebServ::getRessource("./data/default/404.html", *this);
    }
    else 
    {
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        std::cout << "Requesting ressource at path : " << path << std::endl;
        if(!remove(path.c_str()))
        {
            std::cout << "File deleted" << std::endl;
            // TODO : send 200 response without body
        }
        else
        {
            std::cout << "Error deleting file" << std::endl;
            _response.setStatusCode("500");
            _response.setStatusText("Internal Server Error");
            _response.setContentType("text/html");
            WebServ::getRessource("./data/default/500.html", *this);
        }

    }
}

void Request::routingCGI()
{
    if((getFileExtension(path) == "py" ) && (method == "GET" || method == "POST"))
    {
        executable_path = "/usr/bin/python3";
        script_path = path.substr(2);
        CGI cgi(*this);

        cgi.executeCGI(*this);
        _response = cgi.getResponseCGI();
        std::cout << _response.getResponse() << std::endl;
        WebServ::addResponseToQueue(this);
        return;
    }

    if((getFileExtension(path) == "bla" || path == "./file_should_exist_after" ) && (method == "POST"))
    {
        /*CHANGE TO cgi_tester if tested on a MAC*/
        executable_path = "ubuntu_cgi_tester";
        script_path = "";
        CGI cgi(*this);

        cgi.executeCGI(*this);
        _response = cgi.getResponseCGI();
        WebServ::addResponseToQueue(this);
        return;
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
    ready2send = false;
}
