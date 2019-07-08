
# ######################################################################
# First, some housekeeping to determine if we are running on a POSIX
# platform or on Windows

MAKEPROGRAM_EXE=$(findstring exe,$(MAKE))
MAKEPROGRAM_MINGW=$(findstring mingw,$(MAKE))
GITSHELL=$(findstring Git,$(SHELL))
GITSHELL+=$(findstring git,$(SHELL))

# TODO: Remember that freebsd might use not gmake/gnu-make; must add in
# some diagnostics so that user gets a message to install gnu make.

ifneq ($(MAKEPROGRAM_EXE),)
ifeq ($(strip $(GITSHELL)),)
$(error On windows this must be executed from the Git bash shell)
endif
	PLATFORM=Windows
	EXE_EXT=.exe
	LIB_EXT=.dll
	PLATFORM_LDFLAGS=-lmingw32 -lws2_32 -lmsvcrt -lgcc
	HOME=$(HOMEDRIVE)/$(HOMEPATH)
endif

ifneq ($(MAKEPROGRAM_MINGW),)
ifeq ($(strip $(GITSHELL)),)
$(error On windows this must be executed from the Git bash shell)
endif
	PLATFORM=Windows
	EXE_EXT=.exe
	LIB_EXT=.dll
	PLATFORM_LDFLAGS=-lmingw32 -lws2_32 -lmsvcrt -lgcc -lds
	HOME=$(HOMEDRIVE)/$(HOMEPATH)
endif

# If neither of the above are true then we assume a working POSIX
# platform
ifeq ($(PLATFORM),)
	PLATFORM=POSIX
	EXE_EXT=.elf
	LIB_EXT=.so
	PLATFORM_LDFLAGS= -lds
endif



# ######################################################################
# Set the library dir
LIBXCGI=../library

# ######################################################################
# Set the output directories, output filenames

OUTDIR=debug

ifneq (,$(findstring debug,$(MAKECMDGOALS)))
OUTDIR=debug
endif

ifneq (,$(findstring release,$(MAKECMDGOALS)))
OUTDIR=release
endif

VERSION=0.0.1
PROJNAME=pubsub

TARGET=$(shell $(GCC) -dumpmachine)
OUTLIB=$(OUTDIR)/lib/$(TARGET)
OUTBIN=$(OUTDIR)/bin/$(TARGET)
OUTOBS=$(OUTDIR)/obs/$(TARGET)
OUTDIRS=$(OUTLIB) $(OUTBIN) $(OUTOBS)


# ######################################################################
# Declare the final outputs
BINPROGS=\
	$(OUTBIN)/pubsub$(EXE_EXT)

DYNLIB=$(OUTLIB)/lib$(PROJNAME)-$(VERSION)$(LIB_EXT)
STCLIB=$(OUTLIB)/lib$(PROJNAME)-$(VERSION).a
DYNLNK_TARGET=lib$(PROJNAME)-$(VERSION)$(LIB_EXT)
STCLNK_TARGET=lib$(PROJNAME)-$(VERSION).a
DYNLNK_NAME=$(OUTLIB)/lib$(PROJNAME)$(LIB_EXT)
STCLNK_NAME=$(OUTLIB)/lib$(PROJNAME).a


# ######################################################################
# Declare the intermediate outputs
BINOBS=\
	$(OUTOBS)/pubsub.o


OBS=\
	$(OUTOBS)/pubsub_error.o


HEADERS=\
	src/pubsub_error.h


# ######################################################################
# Declare the build programs
ifndef GCC
	GCC=gcc
endif
ifndef GXX
	GXX=g++
endif

CC=$(GCC)
CXX=$(GXX)

COMMONFLAGS=\
	-W -Wall -c -fPIC -I$(HOME)/include\
	-DPLATFORM=$(PLATFORM) -DPLATFORM_$(PLATFORM)\
	-I$(LIBXCGI)/include\
	-DPUBSUB_VERSION=\"$(VERSION)\"


CFLAGS=$(COMMONFLAGS) -std=c99
CXXFLAGS=$(COMMONFLAGS) -std=c++x11
LD=$(GCC)
PRE_LDFLAGS= -L$(HOME)/lib -lm $(PLATFORM_LDFLAGS) -lsqldb -lsqlite3 -lpq
AR=ar
ARFLAGS= rcs


.PHONY:	help real-help show real-show debug release clear-all

# ######################################################################
# All the conditional targets

help: real-help

debug:	CFLAGS+= -ggdb
debug:	CXXFLAGS+= -ggdb
debug:	LDFLAGS+= -L$(LIBXCGI)/debug/lib -lxcgi $(PRE_LDFLAGS)
debug:	all

release:	CFLAGS+= -O3
release:	CXXFLAGS+= -O3
release:	LDFLAGS+= -L$(LIBXCGI)/release/lib -lxcgi $(PRE_LDFLAGS)
release:	all

# ######################################################################
# Finally, build the system

real-help:
	@echo "Possible targets:"
	@echo "help:                This message."
	@echo "show:                Display all the variable values that will be"
	@echo "                     used during execution. Also 'show debug' or"
	@echo "                     'show release' works."
	@echo "debug:               Build debug binaries."
	@echo "release:             Build release binaries."
	@echo "clean-debug:         Clean a debug build (debug is ignored)."
	@echo "clean-release:       Clean a release build (release is ignored)."
	@echo "clear-all:           Clean everything."

real-all:	real-show  $(DYNLIB) $(STCLIB) $(BINPROGS)

all:	real-all
	mkdir -p include
	cp -Rv $(HEADERS) include
	ln -f -s $(STCLNK_TARGET) $(STCLNK_NAME)
	ln -f -s $(DYNLNK_TARGET) $(DYNLNK_NAME)
	ln -f -s $(TARGET)/$(STCLNK_TARGET) $(OUTDIR)/lib/lib$(PROJNAME).a
	ln -f -s $(TARGET)/$(DYNLNK_TARGET) $(OUTDIR)/lib/lib$(PROJNAME)$(LIB_EXT)


real-show:	$(OUTDIRS)
	@echo "SHELL:        $(GITSHELL)"
	@echo "EXE_EXT:      $(EXE_EXT)"
	@echo "LIB_EXT:      $(LIB_EXT)"
	@echo "DYNLIB:       $(DYNLIB)"
	@echo "STCLIB:       $(STCLIB)"
	@echo "CC:           $(CC)"
	@echo "CXX:          $(CXX)"
	@echo "CFLAGS:       $(CFLAGS)"
	@echo "CXXFLAGS:     $(CXXFLAGS)"
	@echo "LD:           $(LD)"
	@echo "LDFLAGS:      $(LDFLAGS)"
	@echo "AR:           $(AR)"
	@echo "ARFLAGS:      $(ARFLAGS)"
	@echo ""
	@echo "PLATFORM:     $(PLATFORM)"
	@echo "TARGET:       $(TARGET)"
	@echo "OUTBIN:       $(OUTBIN)"
	@echo "OUTLIB:       $(OUTLIB)"
	@echo "OUTOBS:       $(OUTOBS)"
	@echo "OUTDIRS:      "
	@for X in $(OUTDIRS); do echo "              $$X"; done
	@echo "HEADERS:      "
	@for X in $(HEADERS); do echo "              $$X"; done
	@echo "OBS:          "
	@for X in $(OBS); do echo "              $$X"; done
	@echo "BINOBS:       "
	@for X in $(BINOBS); do echo "              $$X"; done
	@echo "BINPROGS:     "
	@for X in $(BINPROGS); do echo "              $$X"; done
	@echo "PWD:          $(PWD)"

show:	real-show
	@echo "Only target 'show' selected, ending now."
	@false

$(BINOBS) $(OBS):	$(OUTOBS)/%.o:	src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

$(OUTBIN)/%.exe:	$(OUTOBS)/%.o $(OBS) $(OUTDIRS)
	$(LD) $< $(OBS) -o $@ $(LDFLAGS)

$(OUTBIN)/%.elf:	$(OUTOBS)/%.o $(OBS) $(OUTDIRS)
	$(LD) $< $(OBS) -o $@ $(LDFLAGS)

$(DYNLIB):	$(OBS)
	$(LD) -shared $^ -o $@ $(LDFLAGS)

$(STCLIB):	$(OBS)
	$(AR) $(ARFLAGS) $@ $^

$(OUTDIRS):
	mkdir -p $@

clean-release:
	rm -rfv release

clean-debug:
	rm -rfv debug

clean-all:	clean-release clean-debug


clean:
	echo Choose either clean-release or clean-debug

