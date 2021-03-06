1541img_MODULES:= util log sector track d64 zc45reader filedata \
	hostfilereader hostfilewriter d64writer filename zcfileset \
	zc45extractor cbmdosfile cbmdosvfs cbmdosvfsreader d64reader \
	zc45writer zc45compressor event cbmdosfs cbmdosfsoptions \
	cbmdosinode lynx petscii
ifeq ($(PLATFORM),win32)
1541img_MODULES+= winfopen
endif
1541img_HEADERS_INSTALL:= cbmdosfile cbmdosfileeventargs cbmdosfs \
	cbmdosfsoptions cbmdosvfs cbmdosvfseventargs cbmdosvfsreader d64 \
	d64reader d64writer decl event filedata hostfilereader hostfilewriter \
	log lynx sector track zc45compressor zc45extractor zc45reader \
	zc45writer zcfileset petscii cbmdosinode cbmdosinodeeventargs
1541img_HEADERDIR:= include$(PSEP)1541img
1541img_DEFINES:= -DBUILDING_1541IMG
1541img_CFLAGS_STATIC:= -DSTATIC_1541IMG
1541img_V_MAJ:= 1
1541img_V_MIN:= 1
1541img_V_REV:= 0
1541img_DOCS= README.md LICENSE.txt CHANGES.txt \
	$(wildcard html/search/*) $(filter-out html/search,$(wildcard html/*))
$(call librules, 1541img)

