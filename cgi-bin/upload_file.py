#!/usr/bin/python3

import cgi, os
import cgitb; cgitb.enable()

form = cgi.FieldStorage()

# Get filename here.
fileitem = "Makefile"

# Test if the file was uploaded
#if fileitem.filename:
   # strip leading path from file name to avoid 
   # directory traversal attacks
open(os.getcwd() + '/cgi-bin/' + os.path.basename(fileitem), 'wb').write(fileitem.read())

message = 'The file was uploaded successfully'
   
#else:
 #  message = 'No file was uploaded'
   
print ("<html>")
print ("<head>")
print ("<title>Hello - Second CGI Program</title>")
print ("</head>")
print ("<body>")
print ("<h2>Hello %s </h2>" % (message))
print ("</body>")
print ("</html>")