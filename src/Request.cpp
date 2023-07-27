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
    if (tokens.size() != 3) {
        _response.sendError(400, ": a field from request line is missing"); 
    }

    //std::cout << "REQUEST STRING" << requestString << std::endl;

	parseMethodToken(tokens[0]);
    parseURI(tokens[1]);
	parseHTTPVersion(tokens[2]);
    parseHeaders();
    getRequestConfig();

    path = "." + path;

    if (path == "./")
        path = path + _config->getRoot().substr(1) + "/" + _config->getIndex();

    if(path.find(_config->getName()) != std::string::npos && _config->getType() == "std") {
        path.replace(path.find(_config->getName()), _config->getName().length(), _config->getRoot());
        if (!::fileExists(path) && method == "GET")
            path = path + "/" + _config->getIndex();
    }

    std::cout << " ---------- PARSING COMPLETED ---------- " << std::endl;
    std::cout << "Method: " << method << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "Protocol: " << protocol << std::endl;
    std::cout << "Query: " << query << std::endl;
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

void Request::parseURI(std::string token)
{
	if (token[0] != '/')
        _response.sendError(400, ": URI must begin with a /");
        
    query = "";

	size_t querryChar = token.find("?");
	if (querryChar != std::string::npos)
	{
		path = token.substr(0, querryChar);
		query = token.substr(querryChar + 1, token.size());
	}
	else
		path = token;
}

void Request::parseHTTPVersion(const std::string& token)
{
	if (token.size() < 7 || token.compare(0, 5, "HTTP/") || token.compare(6, 1, ".") || 
			!isdigit(static_cast<int>(token[5])) || !isdigit(static_cast<int>(token[7])))
		_response.sendError(400, ": HTTP version not correct");

	protocol = token;
}

void Request::parseHeaders()
{
    std::string delimiter = "\r\n";
    size_t pos = 0;
    int i = 0;
    std::string tmpRequestString = requestString;
    while ((pos = tmpRequestString.find(delimiter)) != std::string::npos) {
        std::string token = tmpRequestString.substr(0, pos);
        tmpRequestString.erase(0, pos + delimiter.length());
        std::string delimiter2 = ": ";
        size_t pos2 = 0;
        int j = 0;
        while ((pos2 = token.find(delimiter2)) != std::string::npos) {
            std::string token2 = token.substr(0, pos2);
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

void     Request::getRequestConfig()
{
    std::cout << "------- Config Routing ----------" << std::endl;

    std::cout << "request socket fd :" << getServerSocket() << std::endl;
    
    std::vector <VirtualServer*>    matching_servers;
   /* Search matching socket */
    std::vector <VirtualServer*>::iterator    it = Config::getVirtualServers().begin();
    std::vector <VirtualServer*>::iterator    end = Config::getVirtualServers().end();
    while (it != end)
    {
        std::cout << "server socket fd :" << (*it)->getFd() << std::endl;
        if ((*it)->getFd() == getServerSocket())
            matching_servers.push_back(*it);
        it++;
    }
    std::cout << "Matching servers by socket : " << matching_servers.size() << std::endl;
    if(matching_servers.size() == 0)
    {
        std::cout << "No matching server found" << std::endl;
        std::cout << "|-------- End of Config Routing ---------|" << std::endl;
        setConfig(NULL);
        return;
    }
    
    /* Search matching server_name */
    VirtualServer *server;
    it = matching_servers.begin();
    end = matching_servers.end();
    server = *it; // First by default
    while (it != end)
    {
        if ((*it)->getName() == getHeader("Host"))
        {
            server = *it;
            return;
        }
        it++;
    }
    std::cout << "Matching servers by name : " << matching_servers.size() << std::endl;
    
    /* Search matching location */
    std::vector <Location*>  locations = server->getLocations();
    std::vector <Location*>::iterator    locIt;
    std::vector <Location*>::iterator    locEnd;
    std::string                         path = getPath();
    std::string                         location_path;
    locIt = locations.begin();
    locEnd = locations.end();
    std::cout << "locations size " << server->getLocations().size() << std::endl;
    setConfig(server->getLocations()[0]); // first by default
    while(locIt != locEnd)
    {
        std::cout <<  (**locIt) << std::endl;
        location_path = (**locIt).getName();
        std::cout << "location path " << location_path << std::endl;
        std::cout << "request path " << path << std::endl;
        if (path.find(location_path) == 0)
            setConfig(*locIt);
        locIt++;
    }


    std::cout << "|-------- End of Config Routing ---------|" << std::endl;
    // TODO : throw error
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

    _response.setBody(ss.str());
    _response.setStatusCode("200");
    _response.setProtocol("HTTP/1.1");
    _response.setStatusText("OK");
    _response.setContentType("text/html");
    _response.send();
}

void Request::handleRequest() 
{
    // Conf file variables
    std::string root = _config->getRoot(); 
    std::string index = _config->getIndex();


    parseBody();

    //std::cout << "Body: " << body << std::endl;

    std::cout << _config->getName() << std::endl;
    std::cout << _config->getRoot() << std::endl;
    std::cout << _config->getIndex() << std::endl;

    if (path == "./")
        path = path + _config->getRoot().substr(1) + "/" + index;

    //if(_config->getName() == "*.bla")

    if(path.find(_config->getName()) != std::string::npos && _config->getType() == "std") {
        path.replace(path.find(_config->getName()), _config->getName().length(), _config->getRoot());
        //size_t pos = path.find(_config->getRoot(), path.length() - _config->getRoot().length());
        //std::cout << path.substr(path.length() - _config->getRoot().length()) << std::endl;
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

    // If path is a directory, and index file exists, add default index name to path
    /*if (opendir(path.c_str()))
    {
        //path = "." + root + "/" + index;


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
    }*/

    // Modifying URI with root and index directive if any, checking for the allowed methods
    //std::string realUri = reconstructFullURI(_req->getMethod(), loc, _req->getPath());

    // Checking if the targeted file is a CGI based on his extension
    //std::string *cgiName = getCgiExecutableName(realUri, loc.second);

    std::cout << "Path before method routing :" << getPath() << std::endl;

    // Method routing
    if (getFileExtension(path) == "py" || (getFileExtension(path) == "bla" && method == "POST"))
        this->routingCGI();
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
        _response.sendError(501, "Not Implemented");
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
            _response.sendError(500, "Internal Server Error");
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
        _response.setStatusCode("200");
        _response.setStatusText("OK");
        _response.setContentType("text/html");
        _response.setProtocol("HTTP/1.1");
        _response.setBody(cgi.getOutputCGI());
        //ready2send = true;
        _response.send();
        return;
    }

    if((getFileExtension(path) == "bla") && (method == "POST"))
    {
        /*CHANGE TO cgi_tester if tested on a MAC*/
        executable_path = "ubuntu_cgi_tester";
        script_path = "";
        CGI cgi(*this);

        cgi.executeCGI(*this);
        //ready2send = true;
        _response.send();
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
