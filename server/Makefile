# Compiler command line
# Add warning check for implicit data type resolution loss, otherwise
# gcc is silent.
PRE_CC=g++ -m32
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
CC=$(PRE_CC) -Wshorten-64-to-32
endif
ifeq ($(UNAME), Linux)
CC=$(PRE_CC) -Wconversion
endif

LD=$(CC)
CPP_INCS=-I..
OUTDIR=
BINARY=hts
LDFLAGS=-lpthread

# Choose socketserver type

ifeq ($(shell uname -s),Darwin)
SOCKETSERVER=selectserver
endif

ifeq ($(shell uname -s),Linux)
SOCKETSERVER=epollserver
endif

# Debug is default

ifdef REL
CPP_FLAGS=-DRELEASE_LOGGING -g -Wno-write-strings
OUTDIR=release
OPTFLAGS=-O2
else
CPP_FLAGS=-DDEBUG_LOGGING -DDEBUG -DDEV_BUILD -g -Wno-write-strings
OUTDIR=debug
OPTFLAGS=-O0
endif

ifdef DEV_BUILD
CPP_FLAGS=-DDEV_BUILD $(CPP_FLAGS)
endif

CPPFLAGS=$(OPTFLAGS) $(CPP_FLAGS) $(CPP_INCS) -fsigned-char

MAINFILES=main server endpoint room lobby game levelinfo levelinfocache \
ncpackfile ncpdbreader playermgr player tokenauth statsposter httppost \
serverinfoupdater chatlimiter filewatcher tracker badwords tokenbucket \
logger secrets
BASEFILES=deletetracker socket socketaddress socketserver $(SOCKETSERVER) \
bytebuffer eventer md5c messagequeue messagehandler tick format base64 thread
YAJLFILESCPP=jsonbuilder jsontypes
YAJLFILESC=yajl yajl_buf yajl_encode yajl_lex yajl_parser yajl_gen
MPSHAREDFILES=messages netmessage misc xpump ini packfile decompress \
packmanager indexloader xmsglog
INCFILES=rip

ALLFILES=$(INCFILES) $(MAINFILES) $(BASEFILES) $(MPSHAREDFILES) $(YAJLFILESC) \
$(YAJLFILESCPP)

all: $(OUTDIR)/$(BINARY)

clean:
	rm -f $(OUTDIR)/*

$(OUTDIR)/$(BINARY): $(ALLFILES:%=$(OUTDIR)/%.o)
	$(LD) $(LDFLAGS) -o $@ $^ $(MAPFLAGS)

$(MAINFILES:%=$(OUTDIR)/%.o): $(MAINFILES:%=%.cpp)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(BASEFILES:%=$(OUTDIR)/%.o): $(BASEFILES:%=../base/%.cpp)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(MPSHAREDFILES:%=$(OUTDIR)/%.o): $(MPSHAREDFILES:%=../mpshared/%.cpp)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(INCFILES:%=$(OUTDIR)/%.o): $(INCFILES:%=../inc/%.cpp)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(YAJLFILESCPP:%=$(OUTDIR)/%.o): $(YAJLFILESCPP:%=../yajl/wrapper/%.cpp)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(YAJLFILESC:%=$(OUTDIR)/%.o): $(YAJLFILESC:%=../yajl/src/%.c)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).c -o $@

