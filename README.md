# web-server-c
Basic DNS lookup program and web server written to explore socket programming in C. These were created while reading "Computer Systems: A Programmer's Perspective" by Bryant and O'Hallaron.

server.c can serve different clients concurrently. All requests are handled as GET and will prompt the same server response, i.e., "Welcome to Thomas' web server!". The details of the client's HTTP request will also be echoed and printed to the terminal.

tiny_server.c is a whittled-down version of the standard "tiny server" and uses the Rio library from CSAPP for file parsing. It can serve both static and dynamic CGI content to GET requests. The server will assume that its directory of installation is root, and all HTML/GIF/JPG/unformatted text files in that tree can be served to the client. 

server.c and tiny_server.c can be compiled from the command line with gcc or similar, i.e.
`gcc server.c -o server`
then run with the desired port number, i.e.
`./server 9393`
This will start the server on localhost:9393, which can be accessed vie telnet or a browser.
