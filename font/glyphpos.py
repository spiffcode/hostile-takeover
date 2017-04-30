'''
This is a helper script to aid in the mass positioning of font glyphs.

The positioning of the glyph within the image is very important now that the
draws from the idividual glyph images.

The script helps calulate a baseline (the line that capital letters rest upon
when positioned as desired) and then positions most glyphs relative to that.

There's an option to preview the glyph positionings by exporting a font strip,
as well as exporting all the new repositioned glyph images to the disk.
'''

import fontutils
from PIL import Image

# Characters that will rest upon the baseline (excluding hudfont)
CHAR_SET_BASELINE = [
    '!', '?', '.', ':', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
    'L', 'M', 'N', 'O', 'P', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    'h', 'i', 'k', 'l', 'm', 'n', 'o', 'r', 's', 't', 'u', 'v', 'w', 'x', 'z',
]

# Characters that are centered (excluding hudfont)
CHAR_SET_CENTER = [
    '#', '$', '%', '&', '(', ')', '*', '+', '-', '/', '<', '=', '>', '~',
    '@', # galaxite_icon
    '\\' # reactor_icon
]

def get_positioned_glyph(char, font, height, baseline):
    i0 = get_glyph(char, font)
    if i0 == None:
        if font.name != "hudfont": # suppress hudfont errors
            print "Unable to get positioned glyph for \"{}\" in \"{}\"". \
                format(char, font.name)
        return None

    width = i0.size[0]
    if char != ' ':
        width += font.glyph_space

    # Hack: space space out the numbers in hudfont while maintaining
    # hudfont's glyph_space of 0
    if font.name == "hudfont" and char in '0123456789':
        width += fontutils.get_font(art_dir, "standardfont").glyph_space

    i0 = fontutils.crop_whitespace(i0)
    i1 = Image.new('RGBA', (width, height), (0, 0, 0, 0))

    y = calculate_glyph_Y(char, font, height, baseline)
    if y < 0 or y + i0.size[1] > height:
        if char != ' ': # space doesn't matter
            print ("Positioned glyph \"{}\" in \"{}\" is outside font height "
                "bounds: (0,{})".format(char, font.name, y))

    i1.paste(i0, (0, y))
    return i1

def calculate_glyph_Y(char, font, height, baseline):
    i0 = get_glyph(char, font)

    if font.name == "hudfont":
        # All hudfont glpyhs get centered
        return height/2 - i0.size[1]/2

    if char in CHAR_SET_BASELINE:
        return baseline - i0.size[1]
    if char in CHAR_SET_CENTER:
        return height/2 - i0.size[1]/2
    if char == 'Q':
        # Align top with top of standard upercase character
        return calculate_glyph_Y('O', font, height, baseline)
    if char in ['g', 'p', 'q', 'y']:
        # Align top with top of standard lower character
        return calculate_glyph_Y('o', font, height, baseline)
    if char in ['"', '\'']:
        # Theses should be at the top. Anywhere between 0-2 or whatever looks good
        return 0
    if char == 'j':
        # Align top with top of i
        return calculate_glyph_Y('i', font, height, baseline)
    if char == ',':
        # Align top with top of period
        return calculate_glyph_Y('.', font, height, baseline)
    if char == ';':
        # Align top with top of colin
        return calculate_glyph_Y(':', font, height, baseline)
    if char == ' ':
        # Space is invisible so location within image doesn't really matter
        return 0

    print "Error vertically aligning \"{}\"".format(char)
    return 0

def get_glyph(char, font):
    # Returns the glyph image for the passed character
    images = fontutils.images_in_dir(font.path)
    for image in images:
        if fontutils.get_character(image, font) == char:
            return fontutils.crop_whitespace(Image.open("{}/{}".format(font.path, image), 'r'))
    return None

def get_baseline(font, height, baseline_mod):
    # Assume A is the "normal" capital letter height
    im = get_glyph('A', font)
    return height/2 + im.size[1]/2 + baseline_mod

def make_font_strip(strip_string, font, width, height, baseline):
    i0 = Image.new('RGBA', (width, height), (255, 255, 255, 0))
    x = 0
    for char in strip_string:
        i1 = get_positioned_glyph(char, font, height, baseline)
        if i1 == None:
            continue
        i0.paste(i1, (x, 0), i1)
        x += i1.size[0] - font.glyph_overlap
    return i0

def get_perfect_font_height(font, baseline_mod):
    # A perfect font height is when the height is as small as possible while
    # allowing all glyphs positioned correcly without any glyphs being
    # truncated by the vertical edges.

    # Start off by trying the height of the largest trimmed glyph
    height = fontutils.get_font_height(font, True)
    baseline = get_baseline(font, height, baseline_mod)

    # Iterate through each glyph
    images = fontutils.images_in_dir(font.path)
    for image in images:
        char = fontutils.get_character(image, font)
        if char == None or char == ' ':
            continue

        i0 = get_glyph(char, font)
        if i0 == None:
            continue
        i0 = fontutils.crop_whitespace(i0)

        # Keep growing the height until the glyph is no longer truncated
        # when its positioned
        y = calculate_glyph_Y(char, font, height, baseline)
        while y < 0 or y + i0.size[1] > height:
            height += y + i0.size[1] - height
            baseline = get_baseline(font, height, baseline_mod)
            y = calculate_glyph_Y(char, font, height, baseline)

    return height

if __name__ == "__main__":
    # Path to art2432
    art_dir = ''
    # Path to save preview strips
    strip_out_dir = ''
    # String to write in preview
    strip_string = '!"#$%&\'()*+,-./ ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz:;<=>?\@'
    # strip_string = "Now is the time for the quick brown fox to jump over the lazy dog's back!"

    # Modify flag as desired
    # Exports correctly postioned glyphs in a strip for previewing
    export_strips = False

    # Modify flag as desired
    # Overwrites existing font glyphs with correctly positioned glyphs
    export_results = False

    # Iterate over the available fonts

    fonts = fontutils.get_fonts(art_dir)
    for font in fonts:

        # The baseline is calcualted as the line which capital letters rest
        # upon when vertically centered. baseline_mod is a value that's added
        # to that calculation, allowing for letter and number positioning to be
        # tweaked on a per-font basis. Note: baseline_mod other than 0 will
        # cause infinite loop in get_perfect_font_height() if a perfect height
        # cannot be achieved with that baseline
        baseline_mod = 0
        if font.name == "standardfont":
            baseline_mod = -2
        if font.name == "titlefont":
            baseline_mod = -1

        # Minimum height necessary for font whitespace is cropped
        # from all glyphs and no glyps have clipped edges vertically
        height = get_perfect_font_height(font, baseline_mod)

        # The line that capital letters rest upon when  vertically
        # within a perfect height
        baseline = get_baseline(font, height, baseline_mod)

        if export_strips:
            i0 = make_font_strip(strip_string, font, 1200, height, baseline)
            # crop horizontally but not vertically
            if (i0.getbbox() != None):
                i0 = fontutils.crop_horizontal_whitespace(i0)
                i0.save("{}/{}.png".format(strip_out_dir, font.name))
            else:
                print "Failed to save strip for {}".format(font.name)

        if export_results:

            # Iterate over each image
            images = fontutils.images_in_dir(font.path)
            for image in images:

                # Obtain the correctly positioned glyph image
                char = fontutils.get_character(image, font)
                if char == None:
                    print "Error: {}".format(image)
                    continue
                i0 = get_positioned_glyph(char, font, height, baseline)

                # Overwrite original
                i0.save("{}/{}".format(font.path, image))
