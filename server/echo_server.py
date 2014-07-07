#!/usr/bin/env python

"""
A simple echo server
"""

import socket

host = ''
port = 50000
backlog = 5
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((host,port))
s.listen(backlog)
while True:
    client, address = s.accept()
    while True:
        data = client.recv(11)
        if not data:
            break
        client.send(data)
