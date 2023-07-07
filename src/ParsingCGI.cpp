#include "../inc/ParsingCGI.hpp"

ParsingCGI::ParsingCGI() {
    std::cout << "ParsingCGI created" << std::endl;
}

ParsingCGI::~ParsingCGI() {
    std::cout << "ParsingCGI destructed" << std::endl;
}

void ParsingCGI::parseRequest(const std::string& request) {

    // Split the request into lines
    std::istringstream iss(request);
    std::string line;
    getline(iss, line);  // Read the request line

    // Parse the request line
    std::istringstream lineStream(line);
    lineStream >> method >> path >> protocol;

    // Read headers
    while (getline(iss, line) && line != "\r") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);
            headers[headerName] = headerValue;
        }
    }

    // Read the request body
    std::stringstream bodyStream;
    bodyStream << iss.rdbuf();
    body = bodyStream.str();

}

std::string ParsingCGI::getPath() {
    return path;
}

std::string ParsingCGI::getProtocol() {
    return protocol;
}

std::string ParsingCGI::getBody() {
    return body;
}

std::string ParsingCGI::getMethod() {
    return method;
}