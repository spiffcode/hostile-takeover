# Use address sanitizer to find memory usage bugs.
# Install gcc-5/g++-5. To install on Ubuntu trusty:
#
# sudo add-apt-repository ppa:ubuntu-toolchain-r/test
# sudo apt-get update
# sudo apt-get install gcc-5 g++-5

BINARY = HostileTakeover
HTROOT := ../../..
CC = gcc-5
CXX = g++-5
LD = g++-5

LDFLAGS=
CPPFLAGS += -w -Wno-write-strings -fsigned-char \
-I../../../base -I../../../mpshared -I../../../inc \
-I../SDL2/include -I../curl/include -I../../../ -I../../ -I../ \
-D__LINUX__ -DSDL -DMULTIPLAYER -DTRACKSTATE

ifeq ($(REL),1)
OUTDIR = release
CPPFLAGS += -O2
else
OUTDIR = debug
ASAN_FLAGS=-fno-omit-frame-pointer -fno-optimize-sibling-calls
ASAN_FLAGS+=-fsanitize=address # fast mem error detector (heap, stack, global buffer overflow, and use-after free)
#ASAN_FLAGS+=-fsanitize=leak # detect leaks
#ASAN_FLAGS+=-fsanitize=undefined # fast undefined behavior detector
#ASAN_FLAGS+=-fsanitize=float-divide-by-zero # detect floating-point division by zero;
#ASAN_FLAGS+=-fsanitize=bounds # enable instrumentation of array bounds and detect out-of-bounds accesses;
#ASAN_FLAGS+=-fsanitize=object-size # enable object size checking, detect various out-of-bounds accesses.
CPPFLAGS += -g -O0 -DDEV_BUILD -DDEBUG_LOGGING -DDEBUG ${ASAN_FLAGS}
LDFLAGS += ${ASAN_FLAGS}
endif

ifeq ($(shell uname), Darwin)
LDFLAGS += -F/System/Library/Frameworks -framework cocoa -framework foundation \
-F../SDL2/osx -framework SDL2 -fobjc-link-runtime -framework SDL2 \
-rpath ~/Games/HostileTakeover -lcurl -lpthread
CPPFLAGS += -DDARWIN
else
LDFLAGS += -lSDL2 -lcurl -lpthread
endif

# For each source directory:
# wildcard to get all the .cpp or .c files
# notdir to remove the path from the file paths
# basename to remove the suffix from the file name
# filter-out to remove any unwanted source files

GAME_FILES := $(wildcard $(HTROOT)/game/*.cpp)
GAME_FILES := $(notdir $(GAME_FILES))
GAME_FILES := $(basename $(GAME_FILES))
GAME_FILES := $(filter-out SocTransport miscgraphics \
hashtablecode filepdbreader DebugHelpers, $(GAME_FILES))

BASE_FILES := $(wildcard $(HTROOT)/base/*.cpp)
BASE_FILES := $(notdir $(BASE_FILES))
BASE_FILES := $(basename $(BASE_FILES))
BASE_FILES := $(filter-out epollserver stdindispatcher, $(BASE_FILES))

MPSHARED_FILES := $(wildcard $(HTROOT)/mpshared/*.cpp)
MPSHARED_FILES := $(notdir $(MPSHARED_FILES))
MPSHARED_FILES := $(basename $(MPSHARED_FILES))

INC_FILES := rip

YAJL_C_FILES := $(wildcard $(HTROOT)/yajl/src/*.c)
YAJL_C_FILES := $(notdir $(YAJL_C_FILES))
YAJL_C_FILES := $(basename $(YAJL_C_FILES))

YAJL_CPP_FILES := $(wildcard $(HTROOT)/yajl/wrapper/*.cpp)
YAJL_CPP_FILES := $(notdir $(YAJL_CPP_FILES))
YAJL_CPP_FILES := $(basename $(YAJL_CPP_FILES))
YAJL_CPP_FILES := $(filter-out jsonbuilder_test, $(YAJL_CPP_FILES))

SDL_FILES = $(wildcard $(HTROOT)/game/sdl/*.cpp)
SDL_FILES := $(notdir $(SDL_FILES))
SDL_FILES := $(basename $(SDL_FILES))

LINUX_FILES = $(wildcard $(HTROOT)/game/sdl/linux/*.cpp)
LINUX_FILES := $(notdir $(LINUX_FILES))
LINUX_FILES := $(basename $(LINUX_FILES))

all: before_make $(OUTDIR)/$(BINARY)

before_make: 
	@test -d $(OUTDIR) || mkdir -p $(OUTDIR)
	@test -d $(OUTDIR)/game || mkdir -p $(OUTDIR)/game
	@test -d $(OUTDIR)/base || mkdir -p $(OUTDIR)/base
	@test -d $(OUTDIR)/mpshared || mkdir -p $(OUTDIR)/mpshared
	@test -d $(OUTDIR)/inc || mkdir -p $(OUTDIR)/inc
	@test -d $(OUTDIR)/yajl || mkdir -p $(OUTDIR)/yajl
	@test -d $(OUTDIR)/sdl || mkdir -p $(OUTDIR)/sdl
	@test -d $(OUTDIR)/linux || mkdir -p $(OUTDIR)/linux

clean:
	rm -rf $(OUTDIR)/*

$(OUTDIR)/$(BINARY): \
$(GAME_FILES:%=$(OUTDIR)/game/%.o) \
$(BASE_FILES:%=$(OUTDIR)/base/%.o) \
$(MPSHARED_FILES:%=$(OUTDIR)/mpshared/%.o) \
$(INC_FILES:%=$(OUTDIR)/inc/%.o) \
$(YAJL_CPP_FILES:%=$(OUTDIR)/yajl/%.o) \
$(YAJL_C_FILES:%=$(OUTDIR)/yajl/%.o) \
$(SDL_FILES:%=$(OUTDIR)/sdl/%.o) \
$(LINUX_FILES:%=$(OUTDIR)/linux/%.o)
	$(LD) -o $@ $^ $(LDFLAGS)

$(GAME_FILES:%=$(OUTDIR)/game/%.o): $(GAME_FILES:%=../../%.cpp)
	$(CXX) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(BASE_FILES:%=$(OUTDIR)/base/%.o): $(BASE_FILES:%=../../../base/%.cpp)
	$(CXX) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(MPSHARED_FILES:%=$(OUTDIR)/mpshared/%.o): $(MPSHARED_FILES:%=../../../mpshared/%.cpp)
	$(CXX) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(INC_FILES:%=$(OUTDIR)/inc/%.o): $(INC_FILES:%=../../../inc/%.cpp)
	$(CXX) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(YAJL_CPP_FILES:%=$(OUTDIR)/yajl/%.o): $(YAJL_CPP_FILES:%=../../../yajl/wrapper/%.cpp)
	$(CXX) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(YAJL_C_FILES:%=$(OUTDIR)/yajl/%.o): $(YAJL_C_FILES:%=../../../yajl/src/%.c)
	$(CC) $(CPPFLAGS) -c $(<D)/$(*F).c -o $@

$(SDL_FILES:%=$(OUTDIR)/sdl/%.o): $(SDL_FILES:%=../%.cpp)
	$(CXX) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@

$(LINUX_FILES:%=$(OUTDIR)/linux/%.o): $(LINUX_FILES:%=%.cpp)
	$(CXX) $(CPPFLAGS) -c $(<D)/$(*F).cpp -o $@
