import sys
import Image

if __name__ == "__main__":
    i24 = Image.open(sys.argv[1]).convert('RGB')
    width_24 = i24.size[0]
    height_24 = i24.size[1]
    print 'old size: %d, %d' % (width_24, height_24)

    width_32 = width_24 / 3 * 4
    height_32 = height_24 / 3 * 4
    print 'new size: %d, %d' % (width_32, height_32)

    i32 = Image.new('RGB', (width_32, height_32))

    x24 = 0
    y24 = 0
    for y in xrange(height_24):
        for x in xrange(0, width_24, 3):
            avg = (0, 0, 0)
            for j in xrange(3):
                p = i24.getpixel((x+j, y))
                i32.putpixel((x24+j, y24), p)
                avg = (avg[0] + p[0], avg[1] + p[1], avg[2] + p[2])
            i32.putpixel((x24+3, y24), (int(avg[0] / 3), int(avg[1] / 3), int(avg[2] / 3)))
            x24 += 4
        x24 = 0
        y24 += 1
    i32.save('expanded_32.png')
