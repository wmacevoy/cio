VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_PATCH = 0
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

PREFIX    ?= /usr/local
LIBDIR    ?= $(PREFIX)/lib
INCLUDEDIR?= $(PREFIX)/include
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig

CC       ?= cc
CFLAGS   ?= -g
CFLAGS   += -std=c11 -Iinclude
LDLIBS   = -lm -lutf8

UNAME_S := $(shell uname -s)

# Library targets
STATIC_LIB = libcio.a
ifeq ($(UNAME_S),Darwin)
  SHARED_LIB = libcio.$(VERSION_MAJOR).dylib
  SONAME_FLAG = -install_name,$(LIBDIR)/$(SHARED_LIB)
  SOLINK     = libcio.dylib
else
  SHARED_LIB = libcio.so.$(VERSION)
  SONAME     = libcio.so.$(VERSION_MAJOR)
  SONAME_FLAG = -soname,$(SONAME)
  SOLINK     = libcio.so
endif

LIB_SRC = src/cio.c
LIB_OBJ = src/cio.o
LIB_PIC = src/cio.pic.o

# Test target
TEST_BIN = bin/cio_facts
TEST_SRC = src/cio_facts.c
FACTS_SRC = vendor/facts/src/facts.c
TEST_CFLAGS = $(CFLAGS) -Ivendor/facts/include

all: $(STATIC_LIB) $(SHARED_LIB)

$(LIB_OBJ): $(LIB_SRC) include/cio.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB_PIC): $(LIB_SRC) include/cio.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(STATIC_LIB): $(LIB_OBJ)
	$(AR) rcs $@ $<

$(SHARED_LIB): $(LIB_PIC)
	$(CC) -shared -Wl,$(SONAME_FLAG) $(LDFLAGS) -o $@ $< $(LDLIBS)

$(TEST_BIN): $(TEST_SRC) $(STATIC_LIB) include/cio.h
	mkdir -p bin
	$(CC) $(TEST_CFLAGS) $(LDFLAGS) -o $@ $(TEST_SRC) $(FACTS_SRC) $(STATIC_LIB) $(LDLIBS)

.PHONY: check
check: $(TEST_BIN)
	$(TEST_BIN) | diff - expected/cio_facts.out

.PHONY: expected
expected: $(TEST_BIN)
	$(TEST_BIN) >expected/cio_facts.out || true

cio.pc: cio.pc.in
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@LIBDIR@|$(LIBDIR)|g' \
	    -e 's|@INCLUDEDIR@|$(INCLUDEDIR)|g' \
	    -e 's|@PROJECT_VERSION@|$(VERSION)|g' \
	    $< >$@

.PHONY: install
install: $(STATIC_LIB) $(SHARED_LIB) cio.pc
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -d $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(PKGCONFIGDIR)
	install -m 644 include/cio.h $(DESTDIR)$(INCLUDEDIR)/cio.h
	install -m 644 $(STATIC_LIB) $(DESTDIR)$(LIBDIR)/$(STATIC_LIB)
	install -m 755 $(SHARED_LIB) $(DESTDIR)$(LIBDIR)/$(SHARED_LIB)
ifneq ($(UNAME_S),Darwin)
	ln -sf $(SHARED_LIB) $(DESTDIR)$(LIBDIR)/$(SONAME)
endif
	ln -sf $(SHARED_LIB) $(DESTDIR)$(LIBDIR)/$(SOLINK)
	install -m 644 cio.pc $(DESTDIR)$(PKGCONFIGDIR)/cio.pc

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(INCLUDEDIR)/cio.h
	rm -f $(DESTDIR)$(LIBDIR)/$(STATIC_LIB)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_LIB)
ifneq ($(UNAME_S),Darwin)
	rm -f $(DESTDIR)$(LIBDIR)/$(SONAME)
endif
	rm -f $(DESTDIR)$(LIBDIR)/$(SOLINK)
	rm -f $(DESTDIR)$(PKGCONFIGDIR)/cio.pc

# Cosmopolitan Libc (Actually Portable Executable) — static only
.PHONY: cosmo
cosmo:
	$(MAKE) CC=cosmocc AR=cosmoar $(STATIC_LIB)

.PHONY: cosmo-check
cosmo-check:
	$(MAKE) CC=cosmocc AR=cosmoar check

.PHONY: clean
clean:
	rm -f src/*.o $(STATIC_LIB) $(SHARED_LIB) libcio.so* libcio*.dylib cio.pc
	rm -rf bin
