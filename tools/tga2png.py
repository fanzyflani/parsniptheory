"""
Copyright (c) 2014, fanzyflani

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
"""

import sys, struct
import zlib

class TGA:
	def __init__(self, fname):
		fp = open(fname, "rb")

		idlen, cmaptyp, datatyp, cmapbeg, cmaplen, cmapbpp, = struct.unpack("<BBBHHB", fp.read(8))
		ix, iy, iw, ih, ibpp, idesc, = struct.unpack("<hhHHBB", fp.read(10))
		self.iw = iw
		self.ih = ih

		fp.read(idlen)
		self.pal = [tuple(struct.unpack("<BBB", fp.read(3))[::-1]) for i in xrange(cmaplen)]
		self.palused = [0 for i in xrange(256)]
		self.data = [[ord(v) for v in fp.read(iw)] for _ in xrange(ih)]
		if (idesc & 0x20) == 0:
			self.data = self.data[::-1]

		for y in xrange(ih):
			for x in xrange(iw):
				self.palused[self.data[y][x]] += 1

		self.palentsused = sum(map(lambda x : 1 if x != 0 else 0, self.palused))

		fp.close()

class PNGStream:
	def __init__(self, fname):
		self.fp = open(fname, "wb")
		self.fp.write("\x89PNG\x0D\x0A\x1A\x0A")
		self.cnkname = None
		self.cnkbuf = None

	def close(self):
		self.cnk_beg("IEND")
		self.cnk_end()
		self.fp.close()
		self.fp = None

	def cnk_beg(self, name):
		assert len(name) == 4
		assert self.cnkbuf == None
		self.cnkname = name
		self.cnkbuf = name

	def cnk_end(self):
		assert self.cnkbuf != None
		self.fp.write(struct.pack(">I", len(self.cnkbuf)-4))
		self.fp.write(self.cnkbuf)
		self.fp.write(struct.pack(">i", zlib.crc32(self.cnkbuf)))
		self.cnkbuf = None
		self.cnkname = None

	def cnk_write(self, s):
		assert self.cnkbuf != None
		self.cnkbuf += s

# Load image
tga = TGA(sys.argv[1])

# Start writing PNG
fp = PNGStream(sys.argv[2])

# Write IHDR
fp.cnk_beg("IHDR")
fp.cnk_write(struct.pack(">IIBBBBB", tga.iw, tga.ih, 8, 3, 0, 0, 0))
fp.cnk_end()

# Write PLTE
fp.cnk_beg("PLTE")
for (r,g,b) in tga.pal:
	fp.cnk_write(struct.pack(">BBB", r, g, b))
fp.cnk_end()

# Write IDAT
# TODO: Other filter modes
fp.cnk_beg("IDAT")
s = ""
for l in tga.data:
	s += chr(0) + ''.join(chr(v) for v in l)
fp.cnk_write(zlib.compress(s))
fp.cnk_end()

# Close PNG
fp.close()

