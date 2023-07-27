#ifndef CGI_HPP
#define CGI_HPP 

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <cstdlib>
#include <list>

#include "Request.hpp"
#include "Response.hpp"
#include "Utils.hpp"

class CGI {
    private:
        char **_envvar;
		char **_args;
        std::string _exec;
		std::string _realUri;
        std::string extension;
        std::string outputCGI;
        std::string path;

        Response            _response;

    public:
        CGI(Request & req);
		~CGI();
        int pipe_in[2];
        int pipe_out[2];
        std::string _req_body;
	
	
	/* ===================================================================
	 ======================= PUBLIC METHODS ============================*/
		void executeCGI(Request & req);
        Response    getResponseCGI();
};

#endif