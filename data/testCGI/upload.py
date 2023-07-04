#!/usr/bin/python3
import cgi
import os

# Create an instance of the FieldStorage class to parse the request
form = cgi.FieldStorage(keep_blank_values=1)

# Get the uploaded file object
file_item = form['filename']

# Check if the file was successfully uploaded
if file_item.file:
    # Specify the desired location to save the uploaded file
    file_path = '/' + os.path.basename(file_item.filename)

    # Save the uploaded file to the specified location
    with open(file_path, 'wb') as file:
        file.write(file_item.file.read())

    # Generate a response indicating a successful upload
    print('Content-Type: text/html')
    print()
    print('<html>')
    print('<body>')
    print('<h1>File Upload Successful!</h1>')
    print('</body>')
    print('</html>')
else:
    # Generate a response indicating a failed upload
    print('Content-Type: text/html')
    print()
    print('<html>')
    print('<body>')
    print('<h1>File Upload Failed!</h1>')
    print('</body>')
    print('</html>')