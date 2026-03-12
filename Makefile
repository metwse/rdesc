RM = rm -rf
INSTALL ?= install


default: _default

include rdesc.mk

_default: $(RDESC) $(RDESC_SO)

PREFIX ?= /usr/local
install: $(RDESC) $(RDESC_SO)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib/
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include/rdesc/

	$(INSTALL) -m 755 $(RDESC_SO) $(DESTDIR)$(PREFIX)/lib/
	$(INSTALL) -m 644 $(RDESC) $(DESTDIR)$(PREFIX)/lib/

	$(INSTALL) -m 644 $(RDESC_INCLUDE_DIR)/* $(DESTDIR)$(PREFIX)/include/rdesc/

clean:
	$(RM) $(rdesc_DIST_DIR) docs

docs:
	doxygen


.PHONY: default _default clean docs install
