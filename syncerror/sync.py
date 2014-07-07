import simplejson as json
import sys
import re
import os

# Uses a customized version of simplejson, because the dump json has entries
# with the same key, and the order of the keys is important. The customized
# version adds a counter to each key name, for uniqueness, and to recover
# order by sorting. Then at pretty print time, this is removed. Hacky but it
# works.

sys.path.insert(0, '.')

class Frame:
    def __init__(self, d):
        self.d = d

    def GetHash(self):
        keys = self.d.keys()
        keys.sort()
        for key in keys:
            if key[13:] == 'hash':
                return self.d[key]
        return None

    def GetUpdate(self):
        keys = self.d.keys()
        keys.sort()
        for key in keys:
            if key[13:] == 'updates':
                return int(self.d[key])
        return 0

    def PrettyPrint(self):
        # Example: "__00000000__-updates": "9951",
        s = json.dumps(self.d, sort_keys=True, indent=4)
        p = re.compile('^.*(?P<remove>__.*__-).*$')
        lines = []
        for line in s.splitlines():
            line = line.rstrip()
            m = p.match(line)
            if not m:
                lines.append(line)
            else:
                index = line.find(m.group('remove'))
                line2 = line[0:index] + line[index+len(m.group('remove')):]
                lines.append(line2)
        return '\n'.join(lines)

class Dump:
    def __init__(self, filename):
        f = file(filename)
        self.j = json.loads(f.read().rstrip('\x00'))
        f.close()

    def GetFrameNumbers(self):
        numbers = []
        keys = self.j.keys()
        keys.sort()
        for key in keys:
            if key[13:].startswith('frame'):
                numbers.append(int(key[18:]))
        return numbers

    def GetFrame(self, framenumber):
        keys = self.j.keys()
        keys.sort()
        for key in keys:
            if key[13:].startswith('frame'):
                if framenumber == int(key[18:]):
                    return Frame(self.j[key])
        return None

class Differ:
    def __init__(self, file0, file1):
        self.badupdate = -1

        try:
            dump0 = Dump(file0)
            dump1 = Dump(file1)

            for number in dump0.GetFrameNumbers():
                frame0 = dump0.GetFrame(number)
                frame1 = dump1.GetFrame(number)
                if not frame0:
                    break
                if not frame1:
                    break
                if frame0.GetHash() != frame1.GetHash():
                    self.badupdate = frame0.GetUpdate()
                    self.frame0 = frame0
                    self.frame1 = frame1
                    break
        except:
            print 'ERROR loading %s or %s' % (file0, file1)

    def GetBadUpdateNumber(self):
        return self.badupdate

    def Diff(self, frame0file, frame1file, outfile):
        f = file(frame0file, 'w')
        f.write(self.frame0.PrettyPrint())
        f.close()

        f = file(frame1file, 'w')
        f.write(self.frame1.PrettyPrint())
        f.close()

        os.system('diff %s %s > %s' % (frame0file, frame1file, outfile))

if __name__ == '__main__':
    # Find the frame that is not matching

    d = Differ(sys.argv[1], sys.argv[2])

    out0 = '/tmp/foo'
    if len(sys.argv) > 3:
        out0 = sys.argv[3]

    out1 = '/tmp/bar'
    if len(sys.argv) > 4:
        out1 = sys.argv[4]

    d.Diff(out0, out1, '/tmp/diff')

    f = file('/tmp/diff')
    for line in f.read().splitlines():
        print line
    f.close()
