SRCDIR          := src
BINDIR          := bin

FVMBIN          := $(BINDIR)/fvm
DAEMONBIN       := $(BINDIR)/fvm-daemon

FVMHDR          := $(wildcard $(SRCDIR)/*.h)
FVMSRC          := $(wildcard $(SRCDIR)/*.cc)
FVMOBJ          := $(patsubst $(SRCDIR)/%.cc, $(BINDIR)/%.o, $(FVMSRC))
COMMONHDR       := $(wildcard $(SRCDIR)/common/*.h)
COMMONSRC       := $(wildcard $(SRCDIR)/common/*.cc)
COMMONOBJ       := $(patsubst $(SRCDIR)/common/%.cc, $(BINDIR)/common/%.o, $(COMMONSRC))
CONFIGHDR       := $(wildcard $(SRCDIR)/config/*.h)
CONFIGSRC       := $(wildcard $(SRCDIR)/config/*.cc)
CONFIGOBJ       := $(patsubst $(SRCDIR)/config/%.cc, $(BINDIR)/config/%.o, $(CONFIGSRC))
ENCRYPTHDR       := $(wildcard $(SRCDIR)/encrypt/*.h)
ENCRYPTSRC       := $(wildcard $(SRCDIR)/encrypt/*.cc)
ENCRYPTOBJ       := $(patsubst $(SRCDIR)/encrypt/%.cc, $(BINDIR)/encrypt/%.o, $(ENCRYPTSRC))
VCSHDR          := $(wildcard $(SRCDIR)/vcs/*.h)
VCSSRC          := $(wildcard $(SRCDIR)/vcs/*.cc)
VCSOBJ          := $(patsubst $(SRCDIR)/vcs/%.cc, $(BINDIR)/vcs/%.o, $(VCSSRC))
DAEMONHDR       := $(wildcard $(SRCDIR)/daemon/*.h)
DAEMONSRC       := $(wildcard $(SRCDIR)/daemon/*.cc)
DAEMONOBJ       := $(patsubst $(SRCDIR)/daemon/%.cc, $(BINDIR)/daemon/%.o, $(DAEMONSRC))

CC              := g++
LD              := g++
CFLAGS          := $(CFLAGS) -I$(SRCDIR)/ -g -D_FILE_OFFSET_BITS=64 -DDEBUG
LDFLAGS         := $(LDFLAGS) -g
LDADD           := -lrt -lgit2 -lssl -lcrypto
#LDADD           := -lrt -lssl -lssh2 -lgit2 -lfuse

all: fvm

fvm: $(FVMBIN) $(DAEMONBIN)

$(DAEMONBIN): $(DAEMONOBJ) $(COMMONOBJ) $(VCSOBJ) $(CONFIGOBJ)
	$(LD) $(LDFLAGS) $(DAEMONOBJ) $(COMMONOBJ) $(CONFIGOBJ) $(VCSOBJ) -o $(DAEMONBIN) $(LDADD)

$(FVMBIN): $(COMMONOBJ) $(VCSOBJ) $(FVMOBJ) $(CONFIGOBJ) $(ENCRYPTOBJ)
	$(LD) $(LDFLAGS) $(CONFIGOBJ) $(COMMONOBJ) $(VCSOBJ) $(FVMOBJ) $(ENCRYPTOBJ) -o $(FVMBIN) $(LDADD)

$(BINDIR)/common/%.o: $(SRCDIR)/common/%.cc $(COMMONHDR)
	mkdir -p $(BINDIR); mkdir -p $(BINDIR)/common
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR)/config/%.o: $(SRCDIR)/config/%.cc $(COMMONHDR) $(CONFIGHDR)
	mkdir -p $(BINDIR); mkdir -p $(BINDIR)/config
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR)/encrypt/%.o: $(SRCDIR)/encrypt/%.cc $(COMMONHDR) $(ENCRYPTHDR)
	mkdir -p $(BINDIR); mkdir -p $(BINDIR)/encrypt
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR)/vcs/%.o: $(SRCDIR)/vcs/%.cc $(VCSHDR) $(COMMONHDR)
	mkdir -p $(BINDIR); mkdir -p $(BINDIR)/vcs
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR)/daemon/%.o: $(SRCDIR)/daemon/%.cc $(COMMONHDR) $(VCSHDR) $(FVMHDR) $(CONFIGHDR)
	mkdir -p $(BINDIR); mkdir -p $(BINDIR)/daemon
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR)/%.o: $(SRCDIR)/%.cc $(FVMHDR) $(COMMONHDR) $(VCSHDR) $(CONFIGHDR)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

tags:
	cd src; ctags -R --fields=+lS

clean:
	rm -f $(FVMBIN) $(DAEMONBIN) $(FVMOBJ) $(COMMONOBJ) $(VCSOBJ) $(CONFIGOBJ) $(DAEMONOBJ)

