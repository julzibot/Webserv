# Testing Protocol for the toshs_tests.conf File

This is the testing protocol for the `tosh_tests` file.
For each step, I indicate the configuration to test and the expected result. The tests are divided into categories. Each category uses a different route for testing

### 1.1 A redirection route
Server should return a Location for the new Route along with a status code of 301
- **Server Input:**
  Hit a route that has a redirect option in it.
- **Server response:**
  Location as a header along with status of 301.

### 1.2 An autoindexed route
- **Server Input:**
  Hit a route that has autoindexing ON.
- **Expected Output:**
  In case the server is unable to find the file associated with the given location route,
  we should check if Autoindexing is enabled or not.
  If autoindexing is enabled, we can respond with a list of files available in the given
  directory

### 1.3 Methods allowed
- **Server Input:**
  Hit a route with the DELETE method in the case where the methods_except mentions `DELETE`.
- **Expected Output:**
  A 405 response.
- **Explanation:**
  In case a user has mentioned in the configuration that GET and POST are not allowed by using the keywords of `methods_except`, then those routes will not be served by the server.
  The server shall respond with a 405 in this case.
  A 405 HTTP response is for a Method Not Allowed scenario.

### 1.4 Python based CGI route: Working case
- **Server Input:**
  Hit a CGI based route with a GET method.
- **Expected Output:**  
  Python script exeecution with scripts output as body with a status 200 response.
- **Explanation:**
  In case the user tries to access a Route that is meant to respond with a CGI script.
  For instance the basic part of the code works with Python based scripts.
  The response of the script is given back to the server with a 200 response.

### 1.5 Python based CGI route: Flawed script case
- **Expected Output:**  
  In case the user tries to access a CGI route with a script that has syntax issues. Or something that causes the script to fail on execution, in that case, the response of the server shall be a 500 Internal server error due to the flaws in the script.

### 1.5 Python based CGI route: Timeout case
- **Expected Output:**  
  In case the user tries to access a CGI route with a script that runs infinitely long. Then in that case, the server waits until the TIMEOUT duration for the script to run and post that, it cuts off the script execution and responds with a 408 Request Timeout.

### Similar tests can be run for the PHP scripts as well.

### Tests for error cases that we can show users
- 200
- 204
- 301
- 403
- 404
- 405
- 408
- 409
- 413
- 415
- 500