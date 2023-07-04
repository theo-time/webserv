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
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <iostream>
# include "Response.hpp"

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
	
	
	/* ===================================================================
	 ======================= PUBLIC METHODS ============================*/
		void executeCGI();
        Response getResponseCGI();
        std::string getContentType(Request & req);
};

#endif