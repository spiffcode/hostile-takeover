import struct
 
# Reads packed pdb files
#
class Pack:
    def __init__(self, bytes):
        self.bytes = bytes
        self.directory = self.BuildDirectory()

    def BuildDirectory(self):
        bytes = self.GetRecord(0)
        directory = []
        for index in xrange(len(bytes) / 32):
            entrybytes = bytes[index * 32 : index * 32 + 32]
            filename, count, first = struct.unpack('>29sBH', entrybytes)
            directory.append((filename[:filename.find('\0')], first, count))
        return directory

    def Decompress(self, bytes):
        out = ''
        ib = 0
        done = False
        while not done:
            flags, = struct.unpack('B', bytes[ib])
            ib += 1
            for ibit in xrange(7, -1, -1):
                if (flags & (1 << ibit)) != 0:
                    out += bytes[ib]
                    ib += 1
                    continue
                code, = struct.unpack('>H', bytes[ib:ib+2])
                ib += 2
                ibBack = code & 0x1fff
                if (code & 0xe000) != 0:
                    cb = (((code & 0xe000) >> 13) & 0xff) + 1
                else:
                    if ibBack == 0:
                        done = True
                        break;
                    part = struct.unpack('B', bytes[ib])[0] >> 7
                    ibBack = (code << 1) | part
                    cb = (struct.unpack('B', bytes[ib])[0] & 0x7f) + 2;
                    ib += 1
                ibCopy = len(out) - ibBack
                out += out[ibCopy : ibCopy + cb]
        return out

    def GetRecordOffset(self, index):
        offset = 72 + 6 + index * 8
        return struct.unpack('>L', self.bytes[offset:offset+4])[0]

    def GetRecordCount(self):
        offset = 72 + 4
        return struct.unpack('>H', self.bytes[offset:offset+2])[0]

    def GetRawRecord(self, index):
        offset = self.GetRecordOffset(index)
        if (index == self.GetRecordCount() - 1):
            size = len(self.bytes) - offset
        else:
            size = self.GetRecordOffset(index + 1) - offset
        return self.bytes[offset:offset+size]

    def GetRecord(self, index):
        record = self.GetRawRecord(index)
        header = struct.unpack('>3H', record[0:6])
        compressed, cbUncompressed, cbCompressed = header
        if compressed == 0:
            return record[6:]
        else:
            return self.Decompress(record[6:])

    def FindDirectoryEntry(self, filename):
        for entry in self.directory:
            if entry[0] == filename:
                return entry
        return None

    def GetFilenames(self):
        filenames = []
        for entry in self.directory:
            filenames.append(entry[0])
        return filenames

    def GetFile(self, filename):
        entry = self.FindDirectoryEntry(filename)
        if not entry:
            return None
        bytes = ''
        for index in xrange(entry[2]):
            bytes += self.GetRecord(entry[1] + index)
        return bytes

