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
#include "WebServ.hpp"

class CGI {
    private:
        char **_envvar;
		char **_args;
        std::string _exec;
		std::string _realUri;
        std::string extension;
        std::string outputCGI;
        std::string path;
        Request            _req;

    public:
        CGI(Request & req);
		~CGI();
        std::string _req_body;


	
	
	/* ===================================================================
	 ======================= PUBLIC METHODS ============================*/
		bool        executeCGI(Request & req);


        std::string getOutputCGI() {return outputCGI;};


};

#endif