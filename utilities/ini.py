import struct
import cStringIO

#
# Reads binary serialized ini files
#
class Ini:
    def __init__(self, bytes):
        self.bytes = bytes
        self.sections = {}
       
        pos = 0
        while True: 
            nextSection, = struct.unpack('>H', bytes[pos : pos + 2])
            pos += 2
            countProps, = struct.unpack('>H', bytes[pos : pos + 2])
            pos += 2
            nameSection = bytes[pos : pos + bytes[pos:].find('\0')]
            pos += len(nameSection) + 1
            section = []
            for index in xrange(countProps):
                key = bytes[pos : pos + bytes[pos:].find('\0')]
                pos += len(key) + 1
                value = bytes[pos : pos + bytes[pos:].find('\0')]
                pos += len(value) + 1
                section.append((key,value))
            self.sections[nameSection] = section
            if (pos & 1) != 0:
                pos += 1
            if nextSection == 0:
                break

    def __getitem__(self, key):
        return self.sections[key]

    def __str__(self):
        io = cStringIO.StringIO()
        for key in self.sections.keys():
            io.write('%s\n' % key)
            section = self.sections[key]
            for prop in section:
                io.write('\t%s: %s\n' % (prop[0], prop[1]))
            io.write('\n')
        return io.getvalue()
