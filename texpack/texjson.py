import os
import sys
import json

#
# Atlas info
#

# Indexes for side mapped atlases should be ordered in 0-4
ATLAS_INDEX_ANIMATIONS = 0
ATLAS_INDEX_FONTS = 1
ATLAS_INDEX_BITMAPS = 2
ATLAS_INDEX_BITMAPS0 = 3
ATLAS_INDEX_BITMAPS1 = 4
ATLAS_INDEX_BITMAPS2 = 5
ATLAS_INDEX_BITMAPS3 = 6
ATLAS_INDEX_BITMAPS4 = 7
ATLAS_INDEX_UNITS0 = 8
ATLAS_INDEX_UNITS1 = 9
ATLAS_INDEX_UNITS2 = 10
ATLAS_INDEX_UNITS3 = 11
ATLAS_INDEX_UNITS4 = 12

# The names corresponding with the order of the above atlases
ATLAS_NAMES = [
    "animations.png",
    "fonts.png",
    "bitmaps.png",
    "bitmaps0.png",
    "bitmaps1.png",
    "bitmaps2.png",
    "bitmaps3.png",
    "bitmaps4.png",
    "units0.png",
    "units1.png",
    "units2.png",
    "units3.png",
    "units4.png"
]

#
# Packer info
#

# Keep sort types in sync with texpack/Packer.cs
PACKER_SORT_AREA = 0
PACKER_SORT_PERIMETER = 1
PACKER_SORT_WIDTH = 2
PACKER_SORT_HEIGHT = 3
PACKER_SORT_MAX_WH = 4 # max(width, height)

PACKER_DEFAULT_SEARCH_RESOLUTION = 1000

# List of the atlases that will get compiled
PACKERS = [
    ATLAS_INDEX_ANIMATIONS,
    ATLAS_INDEX_FONTS,
    ATLAS_INDEX_BITMAPS,
    ATLAS_INDEX_BITMAPS1,
    ATLAS_INDEX_UNITS1
]

# Keep list in corresponding order with PACKERS
PACKER_ARGS = [
    # args: width, height, search resolution, sorting method
    (420, 420, PACKER_DEFAULT_SEARCH_RESOLUTION, PACKER_SORT_AREA),     # animations
    (250, 250, PACKER_DEFAULT_SEARCH_RESOLUTION, PACKER_SORT_AREA),     # fonts
    (1150, 1000, PACKER_DEFAULT_SEARCH_RESOLUTION, PACKER_SORT_MAX_WH), # bitmaps
    (130, 130, PACKER_DEFAULT_SEARCH_RESOLUTION, PACKER_SORT_AREA),     # bitmaps1
    (2280, 2280, PACKER_DEFAULT_SEARCH_RESOLUTION, PACKER_SORT_AREA)    # units1
]

#
# Side and side map
#

SIDES = [
    0, # side0 (neutral)
    1, # side1 (blue)
    2, # side2 (red)
    3, # side3 (yellow)
    4  # side4 (cyan)
]

# Indexes should be relative to side1 (i.e. side1 is 0)
# See ATLAS_INDEX_ values defined under atlas info
SIDES_ATLAS_INDEX = [
    -1,
    0,
    1,
    2,
    3
]

# The value to hue shift for each side
# Put in -1 for sides that don't need hue shifted
# An estimate equation for the 2432 art is: h(x)=360-230+x
# with x being the desired hue for the art
SIDEMAP_HUES = [
    -1,  # neutral
    -1,  # side1
    130, # side2
    190, # side3
    320  # side4
]

# List of the atlases that will be sidemapped
SIDEMAP_ATLASES = [ATLAS_INDEX_BITMAPS1, ATLAS_INDEX_UNITS1]

# If the atlas is being sidemapped, a grayscale needs to be made for side neutral
GRAYSCALE_ATLASES = SIDEMAP_ATLASES

# Just a global variable to explicitly state that grayscale is for side neutral
GRAYSCALE_SIDE = SIDES[0]

#
# Art info
#

# The dirs of the art going into the animation texture
IMAGE_DIRS_ANIMATIONS = [
    "activator",
    "andyshot",
    "artilleryshot",
    "bullet",
    "movetarget",
    "ricochet",
    "rocket",
    "smoke",
    "tankshot",
    "vacuum",
    "wall",
    "replicator"
]

# The dirs of the art going into the font texture

IMAGE_DIRS_FONTS = [
    "Button font",
    "Hudfont",
    "Shadowfont",
    "Standard font",
    "Title font"
]


# The dirs of the art going into the bitmaps texture
IMAGE_DIRS_BIMAPS = [
    "bitmaps",
    "fog"
]

# The dirs of the art going into the units texture
IMAGE_DIRS_UNITS = [
    "andy",
    "artillery",
    "hq",
    "hrc",
    "lri",
    "ltank",
    "miner",
    "mobilehq",
    "mtank",
    "mtower",
    "proc",
    "radar",
    "reactor",
    "research",
    "rtower",
    "spi",
    "sri",
    "tmac",
    "troc",
    "upgrades",
    "vts",
    "warehouse",
    "sexplosion",
    "vexplosion"
]

# List of all the art dirs
IMAGE_DIRS_ALL = list()
IMAGE_DIRS_ALL.extend(IMAGE_DIRS_ANIMATIONS)
IMAGE_DIRS_ALL.extend(IMAGE_DIRS_FONTS)
IMAGE_DIRS_ALL.extend(IMAGE_DIRS_BIMAPS)
IMAGE_DIRS_ALL.extend(IMAGE_DIRS_UNITS)

# Missing 2432 art is substituted with art from art824
# but this art needs to be treated specially (it needs
# extra processing such as scaling and color mapping).
# Keep the animation section of this list in sync with
# the AMXS_SCALE list in data/makefile so that the makefile
# knows to tell acrunch to scale the animation points
ART_8BIT = [
    # specific images
    "RocketArtifact.png",
    "Rocks.png",
    "Plant.png",
    "Plant1.png",
    "Plant2.png",
    "Plant3.png",
    "Plant4.png",
    "Plant5.png",
    "arrowheaddown.png",
    "arrowheadleft.png",
    "arrowheadright.png",
    "arrowheadup.png",
    "arrow0.png",
    "arrow1.png",
    "arrow2.png",
    "arrow3.png",
    "arrow4.png",
    "arrow5.png",
    "arrow6.png",
    "arrow7.png",
    "x.png",
    # animations
    "movetarget",
    "vacuum",
    "upgrades",
    "tankshot",
    "rocket",
    "ricochet",
    "bullet",
    "andyshot",
    "activator",
    "vts",
    "replicator"
]

# Anything in the above 8bit list that needs to be side mapped
ART_8BIT_SIDEMAP = [
    # specific images
    "arrow0.png",
    "arrow1.png",
    "arrow2.png",
    "arrow3.png",
    "arrow4.png",
    "arrow5.png",
    "arrow6.png",
    "arrow7.png",
    "x.png",
    # animations
    "vts"
]

# The shadow bits in the 824 art need to be mapped to black.
# How dark do we want the shadow?
SHADOW_8BIT = 110

# How much do we scale the 824 art by?
SCALE_8BIT = 1.3333333333333

#
# Script
#

def images_in_dir(dir):
    files = os.listdir(dir)
    for file in files[:]:
        if not file.endswith(".png"):
            files.remove(file)
            continue
        if file.startswith("black_"):
            files.remove(file)
            continue

    return files


if __name__ == "__main__":
    args = sys.argv[1:]

    if len(args) < 1 or len(args) > 2 or args[0] == 'help':
        print "Usage: python texjson.py <art dir> <out json>"
        print "Example: python texjson.py ~/art2432 ~/HostileTakeover/data/art.json"
        sys.exit()

    ART_DIR = args[0]

    # lists for the image objects
    image_entries_animations = list()
    image_entries_fonts = list()
    image_entries_bitmaps = list()
    image_entries_bitmaps1 = list()
    image_entries_units1 = list()

    for dir in IMAGE_DIRS_ALL:
        image_names = images_in_dir("%s/%s" % (ART_DIR, dir))

        for name in image_names:
            entry = {}

            if dir in IMAGE_DIRS_BIMAPS:
                entry['name'] = name
                entry['path'] = "%s/%s" % (dir, name)
            else:
                entry['name'] = "%s/%s" % (dir, name)
                entry['path'] = "%s/%s" % (dir, name)

            if dir in ART_8BIT or name in ART_8BIT:
                entry['black'] = "8bit"
                entry['scale'] = SCALE_8BIT
            else:
                black = "%s/%s/black_%s" % (ART_DIR, dir, name)
                if os.path.isfile(black):
                    entry['black'] = "%s/black_%s" % (dir, name) # relative to art dir
                entry['scale'] = 1.0

            # Does this go in the animations atlas?

            if dir in IMAGE_DIRS_ANIMATIONS:
                image_entries_animations.append(entry)
                continue

            # Does this go in the font atlas?

            if dir in IMAGE_DIRS_FONTS:
                image_entries_fonts.append(entry)
                continue

            # Does this go in thebitmaps atlas? If so, do we put it
            # in the sidemapped or non-sidemapped bitmaps atlas?

            if dir in IMAGE_DIRS_BIMAPS:
                if 'black' in entry:
                    if entry['black'] == "8bit" and not entry['name'] in ART_8BIT_SIDEMAP:
                        # It's 8bit, doesn't have a black, and it's not in the 8bit sidemap list
                        image_entries_bitmaps.append(entry)
                    else:
                        # Has a black, is not 8bit, therefore is is sidemapped
                        # OR
                        # Is 8bit, doesn't have a black, but is explictly listed to be sidemapped
                        image_entries_bitmaps1.append(entry)
                else:
                    # No black, it must be non-sidemapped
                    image_entries_bitmaps.append(entry)
                continue

            # Does this go in the units atlas?

            if dir in IMAGE_DIRS_UNITS:
                image_entries_units1.append(entry)
                continue

    # Iterate over each packer and set the object info

    packer_entries = list()
    for i, packer_index in enumerate(PACKERS):
        packer_entry = {}
        packer_entry['name'] = ATLAS_NAMES[packer_index]
        packer_entry['index'] = packer_index
        packer_entry['width'] = PACKER_ARGS[i][0]
        packer_entry['height'] = PACKER_ARGS[i][1]
        packer_entry['search_resolution'] = PACKER_ARGS[i][2]
        packer_entry['sort_type'] = PACKER_ARGS[i][3]

        # Does this packer need hue variants (i.e. sidemapped)?

        if packer_index in SIDEMAP_ATLASES:
            hue_variants = list()

            for side in SIDES:
                if SIDEMAP_HUES[side] != -1:
                    hue_variant_entry = {}
                    hue_variant_entry['name'] = ATLAS_NAMES[packer_index + SIDES_ATLAS_INDEX[side]]
                    hue_variant_entry['index'] = packer_index + SIDES_ATLAS_INDEX[side]
                    hue_variant_entry['hue'] = SIDEMAP_HUES[side]
                    hue_variants.append(hue_variant_entry)

            if hue_variants:
                packer_entry['hue_variants'] = hue_variants

        # Does this packer need a grayscale?

        if packer_index in GRAYSCALE_ATLASES:
            grayscale_entry = {}
            grayscale_entry['name'] = ATLAS_NAMES[packer_index + SIDES_ATLAS_INDEX[GRAYSCALE_SIDE]]
            grayscale_entry['index'] = packer_index + SIDES_ATLAS_INDEX[GRAYSCALE_SIDE]
            packer_entry['grayscale'] = grayscale_entry

        # Add the appropriate image objects to the appropraite packer

        if packer_index == ATLAS_INDEX_ANIMATIONS:
            packer_entry['images'] = image_entries_animations
        if packer_index == ATLAS_INDEX_FONTS:
            packer_entry['images'] = image_entries_fonts
        if packer_index == ATLAS_INDEX_BITMAPS:
            packer_entry['images'] = image_entries_bitmaps
        if packer_index == ATLAS_INDEX_BITMAPS1:
            packer_entry['images'] = image_entries_bitmaps1
        if packer_index == ATLAS_INDEX_UNITS1:
            packer_entry['images'] = image_entries_units1

        packer_entries.append(packer_entry)

    # Write out the json

    j = {}
    j['art_dir'] = ART_DIR
    j['shadow_8bit'] = SHADOW_8BIT
    j['packers'] = packer_entries

    with open(args[1], 'w') as outfile:
        json.dump(j, outfile, sort_keys = True)
