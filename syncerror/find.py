import glob
import sys
import time
from os.path import join
import sync

def FindFiles(files, file):
    fmt = '%Y.%m.%d-%H.%M.%S'
    seconds = time.mktime(time.strptime(file[11:30], fmt))
    group = []
    for f in files:
        if not f.startswith(file[:8]):
            continue
        s = time.mktime(time.strptime(f[11:30], fmt))
        if abs(s - seconds) >= 60*60:
            continue
        group.append(f)
    group.sort()
    return group

if __name__ == '__main__':
    dumpdir = sys.argv[1]
    outdir = sys.argv[2]
    files = glob.glob1(dumpdir, '????????-?-????.??.??-??.??.??.json')

    while len(files) != 0:
        file = files[0]
        todiff = FindFiles(files, file)
        if len(todiff) < 2:
            print 'Solo file: %s' % file
            files.remove(file)
            continue

        # Diff these files against each other
        for i in xrange(len(todiff) - 1):
            for j in xrange(i + 1, len(todiff)):
                print 'Diffing %s with %s' % (todiff[i], todiff[j])
                d = sync.Differ(join(dumpdir, todiff[i]), join(dumpdir, todiff[j]))
                badupdate = d.GetBadUpdateNumber()
                if badupdate == -1:
                    continue
                print 'Bad Update %d' % badupdate
                str = 'update-%d' % badupdate
                frame0file = todiff[i][:11] + str + '.json'
                frame1file = todiff[j][:11] + str + '.json'
                outfile = todiff[i][:11] + todiff[j][9:11] + str + '.diff'
                d.Diff(join(outdir, frame0file), join(outdir, frame1file), join(outdir, outfile))

        # Now remove these files from the list
        for file in todiff:
            files.remove(file)

