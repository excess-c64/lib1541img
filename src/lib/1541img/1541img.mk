1541img_MODULES:= util log sector track d64 zc45reader filedata hostfilereader \
	d64writer filename zcfileset zc45extractor cbmdosfile cbmdosvfs \
	cbmdosvfsreader d64reader zc45writer zc45compressor event cbmdosfs
1541img_DEFINES:= -DBUILDING_1541IMG
1541img_V_MAJ:= 0
1541img_V_MIN:= 9
1541img_V_REV:= 0
$(call librules, 1541img)

