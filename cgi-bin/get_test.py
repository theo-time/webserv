#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')

#print ("\r\n")
print ("<html>")
print ("<head>")
print ("<title>Hello - Welcome to the GET CGI test</title>")
print ("</head>")
print ("<body>")
print ("<div style='text-align: center; margin-top: 100px;'>")
print ("<h1>Hello %s %s</h1>" % (first_name, last_name))
print ("<p>Thank you for using our service!</p>")
print ("<a href='/../data/correcTest/index.html'>Go back</a>")
print ("</body>")
print ("</html>")