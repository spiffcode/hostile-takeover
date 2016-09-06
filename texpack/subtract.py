import sys
from PIL import Image
import colorsys

def shift_hue(i0, shift):
    width = i0.size[0]
    height = i0.size[1]
    ir = Image.new('RGBA', (width, height))

    for y in xrange(height):
        for x in xrange(width):
            p0 = i0.getpixel((x, y))
            hsl = colorsys.rgb_to_hls(p0[0] / 255.0, p0[1] / 255.0, p0[2] / 255.0)
            huenew = hsl[0] + (shift / 255.0)
            if huenew > 1.0:
                huenew = huenew - 1.0
            rgb = colorsys.hls_to_rgb(huenew, hsl[1], hsl[2])
            rgba = (int(rgb[0] * 255.0), int(rgb[1] * 255.0), int(rgb[2] * 255.0), p0[3])
            ir.putpixel((x, y), rgba)

    return ir

def image_add(i0, i1):
    width = i0.size[0]
    height = i0.size[1]
    ir = Image.new('RGBA', (width, height))

    for y in xrange(height):
        for x in xrange(width):
            p0 = i0.getpixel((x, y))
            p1 = i1.getpixel((x, y))
            r = p0[0] + p1[0]
            if r > 255:
                r = 255
            g = p0[1] + p1[1]
            if g > 255:
                g = 255
            b = p0[2] + p1[2]
            if b > 255:
                b = 255
            a = p0[3]
            ir.putpixel((x, y), (r, g, b, a))

    return ir

if __name__ == "__main__":
    i0 = Image.open(sys.argv[1]).convert('RGBA')
    i1 = Image.open(sys.argv[2]).convert('RGBA')

    width = i0.size[0]
    height = i0.size[1]
    print width, height

    ir = Image.new('RGBA', (width, height))

    for y in xrange(height):
        for x in xrange(width):
            p0 = i0.getpixel((x, y))
            p1 = i1.getpixel((x, y))

            r = p0[0] - p1[0]
            if r < 0:
                r = 0
            g = p0[1] - p1[1]
            if g < 0:
                g = 0
            b = p0[2] - p1[2]
            if b < 0:
                b = 0
            a = p0[3]
            ir.putpixel((x, y), (r, g, b, a))

    bigwidth = width * 16
    bigheight = height * 16
    ibig = Image.new('RGBA', (bigwidth, bigheight))

    hueshift = 0
    for ys in xrange(0, bigheight, width):
        for xs in xrange(0, bigwidth, width):
            inew = shift_hue(ir, hueshift)
            inew = image_add(i1, inew)
            ibig.paste(inew, (xs, ys, xs + width, ys + height))
            print hueshift
            hueshift += 1

    ibig.save('result.png')
