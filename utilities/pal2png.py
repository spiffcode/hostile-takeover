from PIL import Image
import sys

palfile = sys.argv[1]
pngfile = sys.argv[2]

f = open(palfile)
palbytes = f.read()
f.close()

img = Image.new('RGB', (20,256))

for i in xrange(256):
    r = ord(palbytes[2 + i * 3 + 0])
    g = ord(palbytes[2 + i * 3 + 1])
    b = ord(palbytes[2 + i * 3 + 2])

    print 'index: %d == (%d, %d, %d)' % (i, r, g, b)

    for x in xrange(20):
        img.putpixel((x, i), (r, g, b))

img.save(pngfile, format='PNG')
