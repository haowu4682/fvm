SRCDIR          := src
BINDIR          := bin

FVMBIN          := $(BINDIR)/fvm

FVMHDR          := $(wildcard $(SRCDIR)/*.h)
FVMSRC          := $(wildcard $(SRCDIR)/*.cc)
FVMOBJ          := $(patsubst $(SRCDIR)/%.cc, $(BINDIR)/%.o, $(FVMSRC))
COMMONHDR       := $(wildcard $(SRCDIR)/common/*.h)
COMMONSRC       := $(wildcard $(SRCDIR)/common/*.cc)
COMMONOBJ       := $(patsubst $(SRCDIR)/common/%.cc, $(BINDIR)/common/%.o, $(COMMONSRC))
SINGLEHDR       := $(wildcard $(SRCDIR)/single/*.h)
SINGLESRC       := $(wildcard $(SRCDIR)/single/*.cc)
SINGLEOBJ       := $(patsubst $(SRCDIR)/single/%.cc, $(BINDIR)/single/%.o, $(SINGLESRC))

CC              := g++
LD              := g++
CFLAGS          := $(CFLAGS) -I$(SRCDIR)/ -g -D_FILE_OFFSET_BITS=64 -DDEBUG
LDFLAGS         := $(LDFLAGS) -g
LDADD           :=
#LDADD           := -lrt -lssl -lssh2 -lgit2 -lfuse

all: fvm

fvm: $(FVMBIN)

$(FVMBIN): $(FVMOBJ) $(COMMONOBJ) $(SINGLEOBJ)
	$(LD) $(LDFLAGS) $(FVMOBJ) $(SINGLEOBJ) $(COMMONOBJ) -o $(FVMBIN) $(LDADD)

$(BINDIR)/%.o: $(SRCDIR)/%.cc $(FVMHDR) $(COMMONHDR)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR)/common/%.o: $(SRCDIR)/common/%.cc $(COMMONHDR)
	mkdir -p $(BINDIR); mkdir -p $(BINDIR)/common
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINDIR)/single/%.o: $(SRCDIR)/single/%.cc $(SINGLEHDR) $(COMMONHDR)
	mkdir -p $(BINDIR); mkdir -p $(BINDIR)/single
	$(CC) $(CFLAGS) -o $@ -c $<

tags:
	cd src; ctags -R --fields=+lS

clean:
	rm -f $(CLIENT_FUSEBIN) $(CLIENT_FUSEOBJ)

