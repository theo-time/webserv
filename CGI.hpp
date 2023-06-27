#ifndef CGI_HPP
#define CGI_HPP 

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include "Request.hpp"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <sstream>
#include <sys/wait.h>

class CGI {
    private:
        char **_envvar;
		char **_args;
        std::string _exec;
		std::string _realUri;
        std::string extension;
        std::string outputCGI;
        std::string path;

    public:
        CGI(Request *, const std::string &, const std::string& exec = "");
		~CGI();
	
	
	/* ===================================================================
	 ======================= PUBLIC METHODS ============================*/
		void executeCGI();
};

#endif