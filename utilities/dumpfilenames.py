import pack
import sys

filename = sys.argv[1]
print filename
#f = file('%s%s' % ('/Library/WebServer/Documents/wi/pack/', filename))
f = file(filename)
p = pack.Pack(f.read())
f.close()
for f in p.GetFilenames():
    print f

