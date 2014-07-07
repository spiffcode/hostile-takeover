import pack
import sys
import ini
import level

if __name__ == '__main__':
    filename = sys.argv[1]
    f = file(filename)
    p = pack.Pack(f.read())
    f.close()
    ini = ini.Ini(p.GetFile(sys.argv[2]))
    level = level.Level(ini)
    for side in xrange(5):
        print 'Side %d' % side
        triggers = level.GetTriggers(side)
        count = 0
        for trigger in triggers:
            print 'Trigger %d:' % count
            print trigger
            count = count + 1
