#include "Request.hpp"
#include "CGI.hpp"

#include <fstream>


Request::Request(int clientSocket, int serverSocket) : clientSocket(clientSocket), serverSocket(serverSocket) {
    std::cout << "Request created" << std::endl;
    _response.setRequest(this);
}


void Request::parseRequest(){
    std::cout << "--------- REQUEST TO PARSE -------" << std::endl;
    std::cout << requestString << std::endl;
    requestString2 = requestString;
    std::cout << "--------- ++++++++++++++++ -------" << std::endl;
    // Parse first line
    std::string firstLine = requestString.substr(0, requestString.find("\r\n"));
    requestString.erase(0, requestString.find("\r\n") + 2);
    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    int i = 0;
    while ((pos = firstLine.find(delimiter))) {
        token = firstLine.substr(0, pos);
        if (i == 0) {
            method = token;
        } else if (i == 1) {
            path = token;
            originalPath = token;
        } else if (i == 2) {
            protocol = firstLine;
            break;
        }
        firstLine.erase(0, pos + delimiter.length());
        i++;
    }

    // verify method
    if(method == "GET") {
        methodCode = 1;
    } else if (method == "POST") {
        methodCode = 2;
        retrieveHeaderAndBody(requestString2);
    } else if (method == "DELETE") {
        methodCode = 3;
    } else {
        methodCode = 0;
    }

    // verify path
    if (path == "/") {
        path = "/index.html";
    }

    // Parse headers 
    delimiter = "\r\n";
    pos = 0;
    i = 0;
    while ((pos = requestString.find(delimiter)) != std::string::npos) {
        token = requestString.substr(0, pos);
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



    // std::cout << " - PARSING COMPLETED - " << std::endl;
    // std::cout << "Method: " << method << std::endl;
    // std::cout << "Path: " << path << std::endl;
    // std::cout << "Protocol: " << protocol << std::endl;
    // std::cout << "Headers: " << std::endl;
    for (std::map<std::string, std::string>::iterator it=headers.begin(); it!=headers.end(); ++it)
        std::cout << it->first << " => " << it->second << '\n';
}

void Request::retrieveHeaderAndBody(const std::string& input) {
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
}


std::string getFileExtension(std::string filename)
{
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    return extension;
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


Request::~Request() {
    std::cout << "Request destroyed" << std::endl;
}

void Request::get() 
{
    std::cout << "GET" << std::endl;
    std::cout << "Path: " << path << std::endl;
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

void Request::post() 
{
    std::cout << "POST" << std::endl;

    _response.sendError(405, "Method Not Allowed"); // TODO a enlever car juste pour tester
}

void Request::mdelete() 
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

std::string Request::getRedirectionHTML(std::string url)
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
    // --------- PATH PARSING ---------

    // Conf file variables
    std::string root = _config->getRoot(); 
    std::string index = _config->getIndex();


    std::cout << "BODY" << getBody() << std::endl;
    std::cout << "INDEX" << index << std::endl;
    std::cout << "PATH" << path << std::endl;



    if(_config->getName() != "_internal")
        path.replace(path.find(_config->getName()), _config->getName().length(), _config->getRoot());
    else 
        path = root + path;

    // add dot to start of path 
    path = "." + path;

    std::cout << "PATH" << path << std::endl;

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

    std::cout << "Effective path: " << path << std::endl;
    // --------- END OF PATH PARSING ---------

    std::cout << "Handle request" << std::endl;
    std::string fileContent;

    std::cout << getFileExtension(path) << std::endl;


    ParsingCGI requestCGI;

    requestCGI.parseRequest(requestString2);


    if((getFileExtension(path) == "py" ) && (methodCode == GET || methodCode == POST))
    {
        path = "cgi-bin/" + path.substr(2);
        CGI cgi(*this);

        cgi.executeCGI();
        _response = cgi.getResponseCGI(*this);
        WebServ::addResponseToQueue(this);
        return;
    }

    if((getFileExtension(path) == "bla" || path == "./file_should_exist_after" ) && (methodCode == GET || methodCode == POST))
    {
        path = path.substr(2);
        CGI cgi(*this);

        cgi.executeCGI();
        _response = cgi.getResponseCGI(*this);
        WebServ::addResponseToQueue(this);
        return;
    }

    // Method routing
    if (methodCode == GET)
        this->get();
    if(methodCode == POST)
        this->post();
    if(methodCode == DELETE)
        this->mdelete();
    if(methodCode == UNKNOWN)
    {
        if (requestString2.find("HEAD") == 0)
        {
            std::cout << "HEAD" << std::endl;
            std::cout << requestString2 << std::endl;
            _response.sendError(405, "Method Not Allowed");

            /* _response.setStatusCode("405");// TODO a enlever car juste pour tester
            _response.setStatusText("Method Not Allowed");
            _response.setContentType("text/html");
            _response.setProtocol("HTTP/1.1");
            WebServ::getRessource("./data/default/empty.html", *this); */
            return;
        }

        std::cout << "Unknown method" << std::endl;
        _response.setStatusCode("501");
        _response.setStatusText("Not Implemented");
        _response.setContentType("text/html");
        WebServ::getRessource("./data/default/501.html", *this);
    }
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
    return headers[key];
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
    requestString2.clear();
    methodCode = 0;
    cgi_mode = false;
    _config = NULL;
    _response.clear();
}
