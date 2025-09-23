#!/usr/bin/env python3
import http.server
import socketserver
import sys
import os
import socket

if len(sys.argv) < 3:
    print("Usage: python serve_dir.py <port> <directory>")
    sys.exit(1)

port = int(sys.argv[1])
directory = os.path.abspath(sys.argv[2])

if not os.path.isdir(directory):
    print(f"Error: '{directory}' is not a valid directory")
    sys.exit(1)

os.chdir(directory)

class NoCacheHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Disable caching for development
        self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        super().end_headers()

class ReusableTCPServer(socketserver.TCPServer):
    allow_reuse_address = True
    allow_reuse_port = hasattr(socket, "SO_REUSEPORT")

with ReusableTCPServer(("", port), NoCacheHandler) as httpd:
    print(f"Serving '{directory}' at http://localhost:{port}")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down...")
        httpd.shutdown()

