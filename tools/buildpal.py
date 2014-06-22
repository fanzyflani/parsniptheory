"""
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
"""

import struct

def rgb_lerp_chain(sz, l):
	m = [None for _ in xrange(sz)]	

	for i in xrange(len(l)-1):
		i1,r1,g1,b1 = l[i]
		i2,r2,g2,b2 = l[i+1]

		for j in xrange(i1, i2+1, 1):
			if j >= 0 and j < sz:
				k = (j-i1)/float(i2-i1)
				m[j] = tuple(int(0.5+c1+(c2-c1)*k)
					for (c1,c2) in zip((r1,g1,b1),(r2,g2,b2)))

	# This line tends to help.
	return m

files = {
	"player": [ "tga/player.tga" ],
	"tiles": [ "tga/tiles1.tga" ],
	"food": [ "tga/food1.tga" ],
}

cmaps = {}

pranges = {}

pranges["player"] = [
	("trans", 0),
	("exact", 1, 255, 255, 255),
	("exact", 2, 0, 0, 0),
	("map", "pskin", 16, 8),
	("map", "pteam", 24, 8), # Body
	("map", "pteam", 32, 8), # Hair
	("map", "pteam", 40, 8), # Feet
]

pranges["tiles"] = [
	("trans", 0),
	("exact", 1, 255, 255, 255),
	("exact", 2, 0, 0, 0),
	("any", 32, 256-32),
]

pranges["food"] = [
	("trans", 0),
	("exact", 1, 255, 255, 255),
	("exact", 2, 0, 0, 0),
	("any", 32, 256-32),
]

base_prange = [
	("trans", 0),
	("exact", 1, 255, 255, 255),
	("exact", 2, 0, 0, 0),
	("any", 3, 16-3),
	("map", "pskin", 32, 32),
	("map", "pteam", 64, 64),
	("any", 128, 256-128),
]


pmaps = {}
pmap_locs = {}

pmaps["pskin"] = rgb_lerp_chain(32, [
	(0, 255, 255, 255),
	(11, 255, 255, 0),
	(22, 255, 85, 0),
	(31, 85, 31, 0),
])

pmaps["pteam"] = reduce(lambda x, y : x+y, [rgb_lerp_chain(8, [(0, r,g,b), (8, 0,0,0)]) for (r,g,b) in [
	(255, 128, 128),
	(128, 255, 128),
	(128, 128, 255),
	(0, 255, 255),
	(255, 0, 255),
	(255, 128, 64),
	(64, 128, 255),
	(170, 170, 170),
]], [])

class TGA:
	def __init__(self, fname):
		fp = open(fname, "rb")

		idlen, cmaptyp, datatyp, cmapbeg, cmaplen, cmapbpp, = struct.unpack("<BBBHHB", fp.read(8))
		ix, iy, iw, ih, ibpp, idesc, = struct.unpack("<hhHHBB", fp.read(10))

		fp.read(idlen)
		self.pal = [tuple(struct.unpack("<BBB", fp.read(3))[::-1]) for i in xrange(cmaplen)]
		self.palused = [0 for i in xrange(256)]
		self.data = [[ord(v) for v in fp.read(iw)] for _ in xrange(ih)]

		for y in xrange(ih):
			for x in xrange(iw):
				self.palused[self.data[y][x]] += 1

		self.palentsused = sum(map(lambda x : 1 if x != 0 else 0, self.palused))

		fp.close()

print pmaps
anylist = []
anyused = {}
transcol = 255
outpal = [(0, 0, 0) for i in xrange(256)]

# Gather list for "any" type
# Put stuff into "map" slots
for tup in base_prange:
	if tup[0] == "any":
		for i in xrange(tup[2]):
			ni = tup[1]+i
			if ni not in anylist:
				anylist.append(ni)

	elif tup[0] == "trans":
		transcol = tup[1]
	
	elif tup[0] == "exact":
		outpal[tup[1]] = (tup[2], tup[3], tup[4])
	
	elif tup[0] == "map":
		mname = tup[1]
		mstart = tup[2]
		mlen = tup[3]

		p = pmaps[mname]

		for i in xrange(mlen):
			outpal[i + mstart] = p[i]

		pmap_locs[mname] = mstart

# Actually put stuff from the "any" type into the palette
# Also construct colour map for image
for cat in files:
	for fname in files[cat]:
		print "File %s:" % repr(fname)
		tga = TGA(fname)
		print "- Palette entries used: %i" % tga.palentsused

		# Process tuple list
		cmap = [0xFF for i in xrange(256)]
		cmaps[fname] = cmap
		cmaps[fname.replace(".tga", ".img").replace("tga/","dat/")] = cmap
		for tup in pranges[cat]:
			if tup[0] == "trans":
				cmap[tup[1]] = transcol

			elif tup[0] == "exact":
				# TODO: Actually map these out properly
				cmap[tup[1]] = outpal.index((tup[2], tup[3], tup[4]), 1)

			elif tup[0] == "map":
				for i in xrange(tup[3]):
					cmap[tup[2] + i] = pmap_locs[tup[1]] + i

			elif tup[0] == "any":
				for i in xrange(tup[2]):
					ci = tup[1] + i
					if not tga.palused[ci]:
						continue

					cv = tga.pal[ci]
					if cv in anyused:
						# Use the index that's already there.
						cmap[tup[1] + i] = anyused[cv]

					else:
						# Assume we can pop from anylist
						# If we can't, an error happens,
						# which means we have to fix our palettes.
						tv = anylist.pop(0)
						cmap[tup[1] + i] = tv
						outpal[tv] = cv
						anyused[tv] = cv

		print

print "Processing done! %i palette entries free." % len(anylist)
print "Writing pal1"

fp = open("dat/pal1.pal", "wb")

# Write palette data
for i in xrange(256):
	for j in xrange(3):
		fp.write(chr(outpal[i][j]))

# Assemble colour map string table
cmsorder = list(cmaps.iterkeys())
cmsoffs = {}
cmstab = "\x00"
for fname in cmsorder:
	cmsoffs[fname] = len(cmstab)
	cmstab += fname + "\x00"

# Write colour map string table and map
fp.write(chr(len(cmsorder)))
fp.write(struct.pack("<H", len(cmstab)) + cmstab)
for fname in cmsorder:
	fp.write(struct.pack("<H", cmsoffs[fname]))

# Write colour maps
for fname in cmsorder:
	for c in cmaps[fname]:
		fp.write(chr(c))

# Close
fp.close()

