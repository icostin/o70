N:=o70
D:=O70

LIBSRC:=lib.c

DEVDIR:=$(abspath $(or $(DEVDIR),../targets))

TARGET:=$(shell $(CC) -v 2>&1 | grep "^Target: " | sed "s/^Target: //")
TCPFX:=$(CC:%gcc=%)
TCPFX:=$(TCPFX:%clang=%)
TCPFX:=$(TCPFX:%cc=%)
AR:=$(TCPFX)ar
STRIP:=$(TCPFX)strip
CFG:=$(or $(CFG),release)
O:=$(DEVDIR)/$(TARGET)-$(CFG)
B:=/tmp/$(shell whoami)-$(TARGET)-$(CFG)-$N
LIBPFX:=$(if $(findstring mingw,$(CC)),,lib)
LIBEXT:=$(if $(findstring mingw,$(CC)),.dll,.so)
EXEEXT:=$(if $(findstring mingw,$(CC)),.exe,)

LN:=$(LIBPFX)$N$(LIBEXT)
DLIB:=$B/$(LN)
SLIB:=$B/lib$(N).a
DCLI:=$(B)/$(N)
SCLI:=$(B)/$(N)s
CF:=-std=c99 -Wall -Werror -Wextra -fvisibility=hidden -I$(O)/include
LIBCF:=$(CF)
DLIBCF:=$(LIBCF) -fpic -D$D_LIB_BUILD
SLIBCF:=$(LIBCF) -D$D_STATIC

ifeq ($(TARGET),i686-w64-mingw32)
DLIBCF:=$(filter-out -fpic,$(DLIBCF))
endif

CF_debug:=-O0 -D_DEBUG
CF_release:=-O3 -ffast-math -fomit-frame-pointer -DNDEBUG

LIBF:=$(and $(filter $(TARGET),x86_64-linux-gnu),-ffreestanding -nostdlib) -L$(O)/lib -lc42

OK_MSG=[32mOK[0m
FAILED_MSG=[31;1mFAILED[22m
NORMAL_COLOR=[0m
PROJECT_COLOR=[33;1m
TARGET_COLOR=[35;1m
IMSG="$(PROJECT_COLOR)$(N) $(TARGET_COLOR)$(TARGET)-$(CFG)$(NORMAL_COLOR)"

.PHONY: all install clean

all: $(DLIB) $(SLIB) $(DCLI) $(SCLI)

install: all
	mkdir -p $O/include $O/bin $O/lib
	cp -f $(DLIB) $O/lib/
	cp -f $(SLIB) $O/lib/
	cp -f $N.h $O/include/
	cp -f $(DCLI) $O/bin/
	cp -f $(SCLI) $O/bin/
	@echo $(IMSG)

clean:
	-rm -rf $B

$B:
	mkdir -p $@
	chmod 700 $@

$(DLIB): $(patsubst %.c,$B/%-dyn.o,$(LIBSRC)) | $B
	$(CC) -shared -o $@ $^ -Wl,-soname,$(LN) $(LIBF)

$(patsubst %.c,$B/%-dyn.o,$(LIBSRC)): $B/%-dyn.o: %.c $N.h | $B
	$(CC) -c -o $@ $< $(DLIBCF) $(CF_$(CFG))

$(SLIB): $(patsubst %.c,$B/%-sta.o,$(LIBSRC)) | $B
	$(AR) rcs $@ $^

$(patsubst %.c,$B/%-sta.o,$(LIBSRC)): $B/%-sta.o: %.c $N.h | $B
	$(CC) -c -o $@ $< $(SLIBCF) $(CF_$(CFG))

$(DCLI): cli.c $N.h $(DLIB) | $B
	$(CC) -o $@ $< -I. $(CF) $(CF_$(CFG)) -L$O/lib -L$B -l:$(DLIB) -lc42clia -l:$(LIBPFX)c42svc$(LIBEXT) -l:$(LIBPFX)c42$(LIBEXT)

$(SCLI): cli.c $N.h $(SLIB) | $B
	$(CC) -static -o $@ $< -D$D_STATIC -I. $(CF) $(CF_$(CFG)) -L$O/lib -L$B -l:$(SLIB) -lc42clia -lc42svc -lc42

