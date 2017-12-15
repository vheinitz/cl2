<html>
<body>
<?php
/*
# Valentin Heinitz, 2011-12-06
# Simple php upload-script
# POST parameters:
# userfile - name of user file
# uploadDir - directory where new uploaded files are copied to
#             Recomendation for directory name is: <Date>__<Time>_UUID
#             Where Formats for date and time are: yyyy_MM_dd and hh_mm
*/

$UPLOAD_PREFIX = "upload/";
$LOGGING = 1;
$stringData = "";

if ($LOGGING)
{
	$myFile = "./log.txt";
	$fh = fopen($myFile, 'a') or die("can't open file");
	$stringData = "\n\nBegin\n";
	fwrite($fh, $stringData);
}



$uploaddir = $UPLOAD_PREFIX . $_POST['uploadDir'];
if ( ! file_exists( $uploaddir ))
{
	if (!mkdir( $uploaddir, 0777, true ) )
	{
		$stringData = "ERROR: Can't create target directory" . $uploaddir . "\n";
		echo $stringData;
		if($LOGGING)
		{	
		  fwrite($fh, $stringData);
		}
		exit(2);
	}
}

if ( $uploaddir == '' )
{
	$stringData = "ERROR: Upload directory is empty\n";
	echo $stringData;
	if($LOGGING)
	{	
	  fwrite($fh, $stringData);
	}
	exit(2);
}

$uploadfile = $uploaddir . "/". basename($_FILES['userfile']['name']);

if($LOGGING)
{
  $stringData = "UPLOADFILE: " . $uploadfile;
  fwrite($fh, $stringData);
}

echo "<p>";

if (move_uploaded_file($_FILES['userfile']['tmp_name'], $uploadfile)) {
	$stringData =  "File is valid, and was successfully uploaded.\n";
	echo $stringData;
	if($LOGGING)
	{
		fwrite($fh, $stringData);
	}
} else {
	echo "Upload failed\n";
	echo '<pre>';
	echo 'Here is some more debugging info:';
	print_r($_FILES);
	print "</pre>";
	if($LOGGING)
	{
		$stringData =  "\nUpload failed\n";	
		fwrite($fh, $stringData);
	}
}

	if($LOGGING)
	{
		$stringData =  $_FILES['userfile']['tmp_name'];
		fwrite($fh, $stringData);
	}

echo "</p>";

if($LOGGING)
{
	fclose($fh);
}

?> 

</body>
</html>

