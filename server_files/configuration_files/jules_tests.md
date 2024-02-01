# Testing Protocol for the jules.conf File

This is the testing protocol for the `jules.conf` file.
**In ABSOLUTELY NO SCENARIO must the server crash when testing.**
For each step, I indicate the configuration to test and the expected result.
The tests are divided into categories. Each category uses a part of the config file for testing. Comment out all the rest of the file for testing.
**BEFORE STARTING, MAKE SURE THAT THERE ARE AT LEAST 3 HOSTS FOR 127.0.0.1 IN YOUR DEVICE, AND THAT THE 'hosts.types' FILE MATCHES TWO OF THEM** These hosts will be referenced as '[host1]', '[host2]'. In my case, [host1] is "localhost". One of the non-matching hosts will be '[badhost]'


## 1: MISSING INFO

**USE SECTION 1**

### 1.1 comment out all the file.

- **Expected Output:**  
  Server stop -> "Not enough information in config file"

### 1.2 comment out "include hosts.types"

- **Expected Output:**  
  Server stop -> "Not enough information in config file"

### 1.3 comment out all servers.

- **Expected Output:**  
  Server stop -> "Not enough information in config file"

### 1.4 comment out all locations.

- **Expected Output:**  
  Server stop -> "Not enough information in config file"

### 1.5 comment out any 'root' or 'error pages' server directive (not a 'Location' root).

- **Expected Output:**  
  Server stop -> "Config file: 'root' or 'error_pages' missing in server settings"

### 1.6 comment out 'listen' server directive.

- **Expected Output:**  
  Server stop -> "Config file: directive following server is not 'listen'"


## 2: BAD INFO

**USE SECTION 1**

### 2.1 write a bad directive in main directive.

- **Expected Output:**  
  Server stop -> "[bad dir]: Unknown main directive found"

### 2.2 write a bad directive in 'server' directive.

- **Expected Output:**  
  Server stop -> "[bad dir]: Unknown parameter in 'server' directive"

### 2.3 write a bad directive in 'location' directive.

- **Expected Output:**  
  Server stop -> "[bad dir]: Unknown parameter in 'location' directive"

### 2.4 write a bad value for 'max_bodysize'.

- **Expected Output:**  
  PENDING TEST


## 3: BAD FORMAT

**USE SECTION 1**

### 3.1 double 'listen' server directive.

- **Expected Output:**  
  Server stop -> "Config file: 'listen' directive at wrong line"

### 3.2 place a starting server directive ('listen', 'root', 'server_name', 'error_pages') after the 1st 'location' directive.

- **Expected Output:**  
  Server stop -> "Config file: [line]: wrong line place"

### 3.3 place a main directive ('include' or 'max_bodysize') after the 1st 'server' directive..

- **Expected Output:**  
  Server stop -> "Config file: [line]: wrong line place"


## 4: HOST/SERVERNAME: SINGLE SERVER

**USE SECTION 2**
**START WITH SERVER_NAME COMMENTED**

### 4.1 keep the first 'listen', comment out the other one.

- **Expected Output:**  
  The server should work on all the port numbers, for any valid host

### 4.2 keep the second 'listen', comment out the other one.

- **Expected Output:**  
  The server should work on all the port numbers, only for 127.0.0.1 or host names matching this IP ([host1], [host2], ...).

**UNCOMMENT SERVER_NAME. IN THE LINE, CHANGE NAME IF [host1] != "localhost", [host2] != "betahost"**

### 4.3 enter 127.0.0.1 as the request host

- **Expected Output:**  
  The server should work on all the port numbers

### 4.4 enter [host1] as the request host

- **Expected Output:**  
  The server should work on all the port numbers, same as before

### 4.5 enter an IP other than 127.0.0.1

- **Expected Output:**  
  The server should respond with 404

### 4.6 enter [host2] as the request host

- **Expected Output:**  
  The server should respond with 404

### 4.7 enter [badhost] as the request host

- **Expected Output:**  
  The server should respond with 403


## 5: HOST/SERVERNAME: MULTIPLE SERVERS

**USE SECTION 3**
**IN SERVER_NAMES, CHANGE NAMES IF [host1] != "localhost", [host2] != "betahost"**

### 5.1 try '[host1]:9999/youpi'

- **Expected Output:**  
  youpi page

### 5.2 try '[host2]:9999/youpi'

- **Expected Output:**  
  main page

### 5.3 try '[host2]:9999/'

- **Expected Output:**  
  youpi page

### 5.5 try '[host2]:7777/'

- **Expected Output:**  
  main page

### 5.4 try '127.0.0.1:9999/youpi'

- **Expected Output:**  
  youpi page

### 5.6 try '127.0.0.1:8888/youpi'

- **Expected Output:**  
  main page