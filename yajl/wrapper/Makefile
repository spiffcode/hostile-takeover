CC=g++
LD=$(CC)
CPP_INCS=-I../..
OUTDIR=
LDFLAGS=-lpthread
BINARY=jsonbuilder_test

# Debug is default

ifdef REL
CPP_FLAGS=-DLOGGING
OUTDIR=release
OPTFLAGS=-O2
else
CPP_FLAGS=-DLOGGING -DDEBUG -g
OUTDIR=debug
OPTFLAGS=-O0
endif

CPPFLAGS=$(OPTFLAGS) $(CPP_FLAGS) $(CPP_INCS) -fsigned-char

MAINFILES=jsonbuilder_test jsonbuilder jsontypes
YAJLFILES=yajl yajl_buf yajl_encode yajl_lex yajl_parser

ALLFILES=$(YAJLFILES) $(MAINFILES)

all: $(OUTDIR)/$(BINARY)

clean:
	rm -f $(OUTDIR)/*

$(OUTDIR)/$(BINARY): $(ALLFILES:%=$(OUTDIR)/%.o)
	$(LD) $(LDFLAGS) -o $@ $^

$(MAINFILES:%=$(OUTDIR)/%.o): $(MAINFILES:%=%.cpp)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(YAJLFILES:%=$(OUTDIR)/%.o): $(YAJLFILES:%=../src/%.c)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).c -o $@
