NOBUILDTARGETS=html

include zimk/zimk.mk

INCLUDES += -I.$(PSEP)include

$(call zinc, src/lib/1541img/1541img.mk)

html:
	doxygen Doxyfile

clean::
	rm -fr html

distclean::
	rm -fr html

.PHONY: html
