#!/usr/bin/python

import math
import copy
import random
import Image
import ImageDraw
import sys


class RectBase(object):
	def __init__(self):
		self.x = 0
		self.y = 0
		self.width = 0
		self.height = 0
	
	def area(self):
		return self.width * self.height
		
	
class Rectangle(RectBase):
	def __init__(self, width, height, image):
		super(Rectangle,self).__init__()
		
		self.width = width
		self.height = height
		self.image = image
	
	
class Packer:
	def __init__(self, width, height, search_resolution):
		self.rect_list = []
		self.packed_rect_list = []
		self.search_step = 0
		self.area_covered = 0
	
		self.width = width
		self.height = height
		
		self.image = Image.new("RGBA", (width, height))
		self.draw = ImageDraw.ImageDraw(self.image)
		
		self.search_resolution = search_resolution
		
	def add_rect(self, width, height, image):
		self.rect_list.append(Rectangle(width, height, image))
		
	def compile(self):
		# sort rectangles by surface area
		self._sort_rects()
		
		# generate node tree
		#self._build_tree(self.node)
		self._build_tree((0 ,0, self.width, self.height))
		
		# returns list of rectangles that did not fit
		return self.rect_list
	
	def save_image(self, filename):
		a = 0
		for r in self.packed_rect_list:
			#self.draw.rectangle(((r.x, r.y), (r.x + r.width-1, r.y + r.height-1)), fill=r.color, outline=(r.color[0]*2/3, r.color[1]*2/3, r.color[2]*2/3))
			#self.draw.rectangle(((r.x, r.y), (r.x + r.width-1, r.y + r.height-1)), fill=r.color, outline=((255+r.color[0])/2, (255+r.color[1])/2, (255+r.color[2])/2))

			#self.draw.bitmap((r.x, r.y), r.image)
			self.image.paste(r.image, (r.x, r.y, r.x+r.width, r.y+r.height))
			a += r.area()
			
		self.image.save(filename)
		print float(a) / float(self.width * self.height)
		
	def _sort_rects(self):
		self.rect_list.sort(lambda x, y: cmp(x.area(), y.area()), reverse=True)
		#self.rect_list.sort(lambda x, y: cmp(x.area()+(x.width/x.height), y.area()+(y.width/y.height)), reverse=True)
		
	def _build_tree(self, free_space):
		global search_step, search_resolution
		
		if free_space[2] <= 0 or free_space[3] <= 0:
			return
			
		if len(self.rect_list) == 0:
			return
		
		#self.draw.rectangle(((free_space[0], free_space[1]), (free_space[0] + free_space[2], free_space[1] + free_space[3])), fill=None, outline=(255,255,255))
		
		# Find rectangle that fits in current node
		rect_index = 0
		done = False
		step = max(rect_index + len(self.rect_list) / self.search_resolution, 1)
		while not done:
			if (self.rect_list[rect_index].width <= free_space[2]) and (self.rect_list[rect_index].height <= free_space[3]):
				done = True
			else:
				self.search_step += 1
				rect_index += step
				#rect_index = max(rect_index + len(self.rect_list) / 20, 1)
				if rect_index >= len(self.rect_list):
					return
				
		#print rect_index
		
		# Move rectangle from rect_list to packed_rect_list		
		rect = self.rect_list.pop(rect_index)
		self.packed_rect_list.append(rect)
		
		# Set rectangle x, y
		rect.x = free_space[0]
		rect.y = free_space[1]
		
		self.area_covered += rect.width * rect.height
		
		# Determine cutting direction (horizontal or vertical)
		# Split current node
		if (free_space[2] - rect.height) > (free_space[3] - rect.width):
			# cut into two nodes side-by-side
			# Shrink first node of spit nodes
			# call _build_tree for each new node
			#self._build_tree((free_space[0] + rect.width, free_space[1], free_space[2] - rect.width, free_space[3]))
			self._build_tree((free_space[0], free_space[1] + rect.height, rect.width, free_space[3] - rect.height))
			self._build_tree((free_space[0] + rect.width, free_space[1], free_space[2] - rect.width, free_space[3]))
		else:
			# cut into two nodes one on top of the other
			# Shrink first node of spit nodes
			# call _build_tree for each new node
			#self._build_tree((free_space[0], free_space[1] + rect.height, free_space[2], free_space[3] - rect.height))
			self._build_tree((free_space[0] + rect.width, free_space[1], free_space[2] - rect.width, rect.height))
			self._build_tree((free_space[0], free_space[1] + rect.height, free_space[2], free_space[3] - rect.height))


def special_convert(i, is_transparent):
    # Find a tight bounds, create a new image from this
    # Convert magenta to alpha transparent

    w = i.size[0]
    h = i.size[1]

    # top
    t = 0
    stop = False
    for y in xrange(h):
        for x in xrange(w):
            p = i.getpixel((x,y))
            if not is_transparent(p):
                stop = True
                break
        if stop:
            break
        t = y + 1
            
    # left
    l = 0
    stop = False
    for x in xrange(w):
        for y in xrange(h):
            p = i.getpixel((x,y))
            if not is_transparent(p):
                stop = True
                break
        if stop:
            break
        l = x + 1

    # right
    r = w
    stop = False
    for x in xrange(w-1, -1, -1):
        for y in xrange(h):
            p = i.getpixel((x,y))
            if not is_transparent(p):
                stop = True
                break
        if stop:
            break
        r = x - 1
       
    # bottom
    b = h
    stop = False
    for y in xrange(h-1, -1, -1):
        for x in xrange(w):
            p = i.getpixel((x,y))
            if not is_transparent(p):
                stop = True
                break
        if stop:
            break
        b = y - 1

    # crop
    i = i.crop((l, t, r, b))
    w = i.size[0]
    h = i.size[1]

    # convert transparent
    for y in xrange(h):
        for x in xrange(w):
            p = i.getpixel((x,y))
            if is_transparent(p):
                i.putpixel((x,y), (0, 0, 0, 0))

    # convert shadow - black at 40%
    for y in xrange(h):
        for x in xrange(w):
            p = i.getpixel((x,y))
            if p == (156, 212, 248, 255):
                i.putpixel((x,y), (0, 0, 0, 102))

    # resize
    w = w * 32 / 24
    h = h * 32 / 24
    i = i.resize((w,h), resample=1)

    return i

def is_transparent_color(p):
    if p == (255, 0, 255, 255):
        return True
    return False

def is_not_side_color(p):
    if p == (0, 116, 232, 255):
        return False
    if p == (0, 96, 196, 255):
        return False
    if p == (0, 64, 120, 255):
        return False
    if p == (0, 32, 64, 255):
        return False
    return True

def get_hash(i):
    h = 0
    for y in xrange(i.size[1]):
        for x in xrange(i.size[0]):
            p = i.getpixel((x,y))
            v = (p[3] << 24) + (p[0] << 16) + (p[1] << 8) + p[0]
            h = (h + v) ^ (v << 2);
    return h
  
if __name__ == "__main__":
    p = Packer(1024, 1024, 2000)
    f = file(sys.argv[1])
    names = f.read().split('\n')
    f.close()

    imap = {}
    for filename in names:
        if filename == '':
            continue
        i = Image.open(filename).convert('RGBA')
        #i = special_convert(i, is_transparent_color)
        i = special_convert(i, is_not_side_color)
        h = get_hash(i)
        if not imap.has_key(h):
            imap[h] = i
            p.add_rect(i.size[0], i.size[1], i)

    print "Compiling"
    r = p.compile()
    print "rectangles that did not fit:", len(r)
    print [(i.width, i.height) for i in r]
    #for i in r:
    #	print "%i x %i" % (i.width, i.height)
    print "wasted search steps:", p.search_step
    print "area used:", float(p.area_covered) / float(p.width*p.height)

    p.save_image("rect_pack.png")
	
