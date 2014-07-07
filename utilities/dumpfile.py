import pack
import sys

filename = sys.argv[1]
#f = file('%s%s' % ('/Library/WebServer/Documents/wi/pack/', filename))
f = file(filename)
p = pack.Pack(f.read())
f.close()
print p.GetFile(sys.argv[2])
