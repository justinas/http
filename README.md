# HTTP client/server

An HTTP server and client, complete with a handwritten request parser.

## Here be dragons

This code was written for a one-of university project.
It is absolutely not battle-tested 
and was coded to be *good enough* to pass the class.

Use it for inspiration, not production.

## Usage

```
$ make
$ ./client/client 8000
$ ./server/server http://localhost:8000/
```

## Features
Only some of HTTP/1.0 was implemented.

The parser does not allow any other methods than GET.
It does parse the path and headers.

The server lists files in the current directory
and lets the user download them.
It may or may not be susceptible to path-traversal.

The client lets you issue an HTTP/1.0 GET request.
That's about it.
