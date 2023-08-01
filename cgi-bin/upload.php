<!DOCTYPE html>
<html>
<head>
    <title>File Upload</title>
</head>
<body>
    <h2>File Upload</h2>
    <?php
    if ($_SERVER["REQUEST_METHOD"] === "POST" && isset($_FILES["uploaded_file"])) {
    $target_dir = $_SERVER["DOCUMENT_ROOT"] . "/data/upload_dir/";
    $target_file = $target_dir . basename($_FILES["uploaded_file"]["name"]);
    $upload_ok = true;
    $file_type = strtolower(pathinfo($target_file, PATHINFO_EXTENSION));

    // Check if the file is an actual file or a fake file
    if ($_FILES["uploaded_file"]["size"] == 0) {
        echo "Error: No file selected.";
        $upload_ok = false;
    }
    //echo $_FILES["uploaded_file"]["size"];
    // Check if the file already exists
    if (file_exists($target_file)) {
        echo "Error: File already exists.";
        $upload_ok = false;
    }

    // Allow certain file formats (you can customize this list)
    $allowed_formats = array("cpp", "txt", "html", "css");
    if (!in_array($file_type, $allowed_formats)) {
        echo "Error: Only CPP, HTML, CSS and TXT files are allowed.";
        $upload_ok = false;
    }

    if ($upload_ok) {
        if (move_uploaded_file($_FILES["uploaded_file"]["tmp_name"], $target_file)) {
            echo "File uploaded successfully: " . htmlspecialchars(basename($_FILES["uploaded_file"]["name"]));
        } else {
            echo "Error uploading file.";
        }
    }
}
    ?>

    <form action="upload.php" method="post" enctype="multipart/form-data">
        Select file to upload:
        <input type="file" name="uploaded_file" id="uploaded_file">
        <input type="submit" value="Upload File" name="submit">
    </form>

    <a href='/../data/correcTest/index.html'>Go back</a>
</body>
</html>