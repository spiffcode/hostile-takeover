import os
from PIL import Image

SUFFIX = ".png"

FONT_DIR_NAMES = [
    "Button font",
    "Hudfont",
    "Shadowfont",
    "Standard font",
    "Title font"
]

SPECIAL_CHARS = [
    ('and symbol', '&'),
    ('apostrophe', '\''),
    ('astrex symbol', '*'),
    ('colon', ':'),
    ('comma', ','),
    ('dash', '-'),
    ('dollar sign', '$'),
    ('equal symbol', '='),
    ('exclamation mark', '!'),
    ('exlamation point', '!'),
    ('forward slash', '/'),
    ('galaxite_icon', '@'),
    ('number symbol', '#'),
    ('percentage', '%'),
    ('period', '.'),
    ('plus', '+'),
    ('question mark', '?'),
    ('reactor_icon', '\\'),
    ('semicolon', ';'),
    ('at symbol', '@'),
    ('dollar symbol', '$'),
    ('equal sign', '='),
    # ('multiply symbol', 'X'),
    ('quotation mark', '"'),
    ('quotation marks', '"'),
    ('zero_null', '~'),
    # ('empty sum symbol', None), # u'\u2205'
    ('left arrow bracket', '<'),
    ('right arrow bracket', '>'),
    ('left bracket', '('),
    ('right bracket', ')'),
    ('left bracket ', '('),
    ('right bracket ', ')'),
    ('left parenthesis', '('),
    ('right parenthesis', ')'),
    ('space', ' '),
    # These are only for hudfont
    ('currency_bar_left', 'A'),
    ('currency_bar_mid', 'B'),
    ('currency_bar_right', 'C'),
    ('power_bar_half', 'D'),
    ('power_bar_mid', 'E'),
    ('power_bar_right', 'F'),
    ('power_bar_arrows', 'H'),
    ('power_bar_half', 'L'),
    ('power_bar_tick_green', 'I'),
    ('power_bar_tick_red', 'K'),
    ('power_bar_tick_yellow', 'J'),
    ('power_symbol', 'G')
]

class Font():
    def __init__(self, name, dir_name, path, default_char, glyph_overlap, line_overlap, glyph_space):
        self.name = name # font name for glyph filename notation
        self.dir_name = dir_name # name of font directoy
        self.path = path # path to font directory (for accessing the files)
        self.default_char = default_char # char to draw if the requested char isn't found
        self.glyph_overlap = glyph_overlap
        self.line_overlap = line_overlap
        self.glyph_space = glyph_space # amount of whitespace to be added to the right of the glyph within the image

def get_fonts(art_dir):
    fonts = list()
    # Modify these as necessary
    fonts.append(Font("buttonfont", "Button font",
        "{}/Button font" .format(art_dir), "?", 0, 0, 1))
    fonts.append(Font("hudfont", "Hudfont",
        "{}/Hudfont" .format(art_dir), "0", 0, 0, 0))
    fonts.append(Font("shadowfont", "Shadowfont",
        "{}/Shadowfont" .format(art_dir), "?", 1, 2, 0))
    fonts.append(Font("standardfont", "Standard font",
        "{}/Standard font" .format(art_dir), "?", 0, 0, 1))
    fonts.append(Font("titlefont", "Title font",
        "{}/Title font" .format(art_dir), "?", 0, 0, 2))
    return fonts

def images_in_dir(dir):
    files = os.listdir(dir)
    for file in files[:]:
        if not file.endswith(SUFFIX):
            files.remove(file)
            continue
        '''
        if file.startswith("black_"):
            files.remove(file)
            continue
        '''
    return files

def parse_special_char_name(name):
    for entry in SPECIAL_CHARS:
        if entry[0] == name:
            return entry[1]
    return None

def get_character(im_name, font):
    # Naming notations... In all cases, [char] should have a length of 1
    #
    # Normal notation: [fontname]_[char].png
    # char will be mapped to this image
    #
    # Lowercase notation: [fontname]_lowercase_[char].png
    # the lower case of char will be mapped to the image (this implies that
    # char should be a letter)
    #
    # Underscore1 notation: [fontname]_[char]_1.png
    # another notation for lowercase
    #
    # Special notation: [fontname]_[special].png
    # special can be any string that's mapped in SPECIAL_CHARS

    char = im_name
    if not char.startswith(font.name):
        return None
    if not char.endswith(SUFFIX):
        return None

    # Strip [fontname]_ and [suffix]
    char = char.replace("{}_".format(font.name), "")
    char = char.replace(SUFFIX, "")

    # Parsing error or image name not valid font notation
    if len(char) == 0:
        return None

    # Normal notation
    if len(char) == 1:
        return char

    # Could be lowercase notation, underscore1 notation, special notation,
    # or an error
    if len(char) > 1:

        # Lowercase notation
        if char.startswith("lowercase_"):
            char = char.replace("lowercase_", "")
            if len(char) != 1:
                return None
            char = char.lower()
            return char

        # Underscore1 notation
        if char.endswith("_1"):
            char = char.replace("_1", "")
            if len(char) != 1:
                return None
            char = char.lower()
            return char

        # Special notation or error
        char = parse_special_char_name(char)
        if char == None:
            return None
        return char

    # Execution shouldn't ever get here, but just in case
    return None

def get_font_height(font, crop):
    # Returns height of tallest image in font folder
    # Pass crop=True to crop whitespace before height comparison
    images = images_in_dir(font.path)
    height = 0
    for image in images:
        im = Image.open("{}/{}".format(font.path, image), 'r')
        if crop:
            im = crop_whitespace(im)
        if im.size[1] > height:
            # Ensure that this image is valid for the font
            if get_character(image, font) != None:
                # We don't care about space's height
                if get_character(image, font) != ' ':
                    height = im.size[1]
                    #print "{} {}".format(height, image)
    return height

def crop_whitespace(i0):
    # getbbox() only trims 0,0,0,0 pixels
    # Convert all pixels with 0 alpha to 0,0,0,0
    pixels = i0.load()
    newData = []
    for y in range(i0.size[1]):
        for x in range(i0.size[0]):
            pixel = pixels[x, y]
            if pixel[3] == 0:
                newData.append((0, 0, 0, 0))
            else:
                newData.append(pixel)
    i0.putdata(newData)
    return i0.crop(i0.getbbox())

def crop_horizontal_whitespace(i0):
    # getbbox() only trims 0,0,0,0 pixels
    # Convert all pixels with 0 alpha to 0,0,0,0
    pixels = i0.load()
    newData = []
    for y in range(i0.size[1]):
        for x in range(i0.size[0]):
            pixel = pixels[x, y]
            if pixel[3] == 0:
                newData.append((0, 0, 0, 0))
            else:
                newData.append(pixel)
    i0.putdata(newData)
    return i0.crop((0, 0, i0.getbbox()[2], i0.size[1]))

def get_font(art_dir, font_name):
    fonts = get_fonts(art_dir)
    for font in fonts:
        if font.name == font_name:
            return font
    return None
