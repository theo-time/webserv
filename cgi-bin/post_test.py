#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
first_name = form.getvalue('first_name')
last_name  = form.getvalue('last_name')

print ("<html>")
print ("<head>")
print ("<title>Hello - Welcome to the POST CGI test</title>")
print ("</head>")
print ("<body>")
print ("<div style='text-align: center; margin-top: 100px;'>")
print ("<h2>Hello %s %s</h2>" % (first_name, last_name))
print ("<p>Thank you for using our service!</p>")
print ("<a href='/../data/correcTest/index.html'>Go back</a>")
print ("</body>")
print ("</html>")