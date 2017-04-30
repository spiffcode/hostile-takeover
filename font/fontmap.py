import sys
import json
import fontutils

if __name__ == "__main__":

    art_dir = sys.argv[1]
    out_dir = sys.argv[2]

    fonts = fontutils.get_fonts(art_dir)
    errors = list()

    for font in fonts:
        glyph_map = {}
        images = fontutils.images_in_dir(font.path)
        for image in images:
            # image_name that the character will be mapped to
            # this is the name the game will use to load the glyph
            image_name = "{}/{}".format(font.dir_name, image)

            # Magically get the chracter name
            char = fontutils.get_character(image, font)
            if char == None:
                errors.append(image)
                continue

            # Different characters can be mapped to the same glyph, but you
            # can't have the same character mapped to different glyphs in the
            # same font
            if char in glyph_map:
                print ("{}: trying to add \"{}\" with key \"{}\" but"
                    "\"{}\" already has been mapped to \"{}\""
                    .format(font.name,image_name, char, char, glyph_map[char]))
            # Map it
            glyph_map[char] = image_name

        # It's important that the default character exists becuase it is drawn
        # whenever a character can't be found in the map
        if font.default_char not in glyph_map:
            # If this is happening, make sure an image with the appropriate
            # notation exists in the font's dir
            print ("Unable to write {}.json: deafult char \"{}\""
                "doesn't exist in map.".format(font.name, font.default_char))
            continue

        # Construct the object
        j = {}
        j['default'] = glyph_map[font.default_char]
        j['height'] = fontutils.get_font_height(font, False)
        j['glyph_overlap'] = font.glyph_overlap
        j['line_overlap'] = font.line_overlap
        j['glyph_map'] = glyph_map

        with open("{}/{}.json".format(out_dir, font.name), 'w') as outfile:
            json.dump(j, outfile, sort_keys = True)

    if len(errors) > 0:
        print "Unprocessed images:"
        for fn in errors:
            print fn
