# web-server-c
Basic DNS lookup program and web server written to explore socket programming in C. These were created while reading "Computer Systems: A Programmer's Perspective" by Bryant and O'Hallaron.

server.c can serve different clients concurrently. All requests are handled as GET and will prompt the same server response, i.e., "Welcome to Thomas' web server!". The details of the client's request will also be echoed and printed to the terminal.

server.c can be compiled from the command line with gcc or similar, i.e.
`gcc server.c -o server`
then run with the desired port number, i.e.
`./server 9393`
This will open a server on localhost:9393, which can be accessed via telnet from the command line or a browser.
