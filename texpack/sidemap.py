import sys
import Image
import colorsys

class Sidemap:
    def __init__(self):
        pass

    def shift_hue(self, i0, shift):
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

    def image_add(self, i0, i1):
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

    def image_subtract(self, i0, i1):
        width = i0.size[0]
        height = i0.size[1]
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
        return ir

    def gen_unit_image(self, white, black, hue):
        sidepixels = self.image_subtract(white, black)
        sidecolors = self.shift_hue(sidepixels, hue)
        return self.image_add(black, sidecolors)

    def shift_file(self, whitefile, blackfile, hue, outfile):
        white = Image.open(whitefile).convert('RGBA')
        black = Image.open(blackfile).convert('RGBA')
        result = self.gen_unit_image(white, black, hue)
        result.save(outfile)
