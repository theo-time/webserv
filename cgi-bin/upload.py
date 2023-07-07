#!/usr/bin/python3

# Import modules for CGI handling 
import cgi

import cgitb
cgitb.enable(display=0, logdir=None, context=1)

# Create instance of FieldStorage 
form = cgi.FieldStorage()

# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')

print ("<html>")
print ("<head>")
print ("<title>Hello - Second CGI Program</title>")
print ("</head>")
print ("<body>")
print ("<h2>Hello %s %s</h2>" % (first_name, last_name))
print ("</body>")
print ("</html>")