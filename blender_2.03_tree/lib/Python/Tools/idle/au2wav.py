import sys
import os
import sunau
import StringIO
import wave
import winsound

def findfile(fn):
    for dir in sys.path:
        fullname = os.path.join(dir, fn)
        try:
            if os.path.exists(fullname):
                return fullname
        except IOError:
            continue
    return fn

fn = "test/audiotest.au"
fn = findfile(fn)
print fn
fp = sunau.open(fn, 'r')
print fp._encoding

params = fp.getparams()
print params
nchannels, sampwidth, framerate, nframes, comptype, compname = params
comptype, compname = 'NONE', "No compression"

ofn = "temp.wav"
ofp = open(ofn, "wb")
wavefp = wave.Wave_write(ofp)
wavefp.setparams((nchannels, sampwidth, framerate, nframes, comptype, compname))

bufsize = 8192

while 1:
    data = fp.readframes(bufsize)
    if not data:
        break
    print "read", len(data), "bytes"
    wavefp.writeframesraw(data)

wavefp.close()
ofp.close()

##print ofp.buflist
##alldata = ofp.getvalue()
##print len(alldata)
##winsound.PlaySound(alldata, winsound.SND_MEMORY)

winsound.PlaySound(ofn, winsound.SND_FILENAME)
