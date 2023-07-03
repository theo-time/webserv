#!/usr/bin/env python3
import cgi
import os

# Set the upload folder
UPLOAD_FOLDER = '/tmp'

# Create an instance of the FieldStorage class to get the uploaded file
form = cgi.FieldStorage()

# Check if the file field is present in the form
if 'filename' not in form:
    print("No file uploaded")
else:
    fileitem = form['filename']

    # Check if the file was uploaded successfully
    if fileitem.filename:
        # Get the filename and path
        filename = os.path.basename(fileitem.filename)
        filepath = os.path.join(UPLOAD_FOLDER, filename)

        # Save the file to the upload folder
        with open(filepath, 'wb') as f:
            f.write(fileitem.file.read())

        print("File uploaded successfully")
    else:
        print("No file uploaded")