import pack
import ini
import sys

filename = sys.argv[1]
f = file(filename)
p = pack.Pack(f.read())
f.close()
ini = ini.Ini(p.GetFile(sys.argv[2]))
print ini
