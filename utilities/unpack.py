import pack
import sys

filename = sys.argv[1]
f = file(filename)
p = pack.Pack(f.read())
f.close()
for filename in p.GetFilenames():
    f = open(filename, 'w')
    f.write(p.GetFile(filename))
    f.close()
