#!/usr/bin/python

import os

print ("Content-type: text/html\r\n\r\n")
print ("<font size=+1>Environment</font><\br>")
exec_file = os.environ['PATH_INFO']
print(exec_file)
print("</body>\r")
print("</html>\r")