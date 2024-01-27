<?php

// Get the request method from the first command line argument
$method = strtoupper($argv[1]);

if ($method == 'GET') {
    // Get all .record files in the current directory
    foreach (glob("*.record") as $filename) {
        // Read and print the contents of each file
        $student = json_decode(file_get_contents($filename), true);
        print_r($student);
    }
} elseif ($method == 'POST') {
    // Parse the student record from the second command line argument
    $student = json_decode($argv[2], true);

    // Store the student record in a file named "{id}.record"
    file_put_contents("{$student['id']}.record", json_encode($student));
} else {
    echo "Invalid request method";
}

?>