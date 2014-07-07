#!/usr/bin/env python

"""
A simple echo client
"""

import socket
import time

#host = '174.129.126.177'
host = '79.125.42.223'
port = 50000
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host,port))

while True:
    start = time.time()
    s.send('hello world')
    data = s.recv(11)
    end = time.time()
    print 'RTT: %dms' % int((end - start) * 1000)

s.close()
