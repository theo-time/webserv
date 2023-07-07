#include "inc/ParsingCGI.hpp"

int main() {
    std::string httpRequest = "POST /path/to/resource HTTP/1.1\r\n"
                              "Host: example.com\r\n"
                              "Content-Type: application/json\r\n"
                              "\r\n"
                              "{\"key\":\"value\"}";

    ParsingCGI req;

    req.parseRequest(httpRequest);

    // Access the parsed request components
    std::cout << "Method: " << req.getMethod() << std::endl;
    std::cout << "Path: " << req.getPath() << std::endl;
    std::cout << "Protocol: " << req.getProtocol() << std::endl;
    std::cout << "Headers:" << std::endl;
    for (const auto& header : req.headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }
    std::cout << "Body: " << req.getBody() << std::endl;

    return 0;
}