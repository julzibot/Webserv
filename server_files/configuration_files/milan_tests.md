# Sockets, POST and DELETE

## Sockets

 - Overload the server
	 - siege -b http://localhost:9999/empty_page
## POST
 - [ ] **We need to create an img folder!**
 - Post image
 - Post text
 - POST an existing image (difficult to do because of timestamp)
	 - This is difficult to do due to timestamp, but works when an image is small, Bjork image works to test this
 - **CGI** POST
 - POST without the body
 - POST using multipart/form-data
 - POST a simple file (binary)
 - POST an image beyond max_body_size
## DELETE
 - DELETE a POSTed image
 - DELETE without a path (empty DELETE)
 - DELETE from a previous folder (localhost:9999/../some_file)
 - DELETE a file that does not exist
 - DELETE invalid path

 ## CROSS-TESTING
 - CGI - bad CGI POST input
 - bad root POST