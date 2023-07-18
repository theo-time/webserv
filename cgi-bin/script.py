#!/usr/bin/python

first_name = "Juan"
last_name  = "Braco"

#print ("Content-type:text/html\r\n")




#print ("\r\n")
print ("<html>")
print ("<head>")
print ("<title>Hello py cgi</title>")
print ("</head>")
print ("<body>")
print ("<div style='text-align: center; margin-top: 100px;'>")
print ("<h1>Hello %s %s</h1>" % (first_name, last_name))
print ("<p>Thank you for using our service!</p>")
print ("<a href='/data/testCGI/index.html'>Go back</a>")
print ("</body>")
print ("</html>")