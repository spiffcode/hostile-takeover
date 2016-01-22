TOOLDIR=../bin
CPP_FLAGS=-Wno-multichar -Wno-write-strings

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
BINARY=inicrunch_osx
endif
ifeq ($(UNAME), Linux)
BINARY=inicrunch_linux
endif

all:
	g++ $(CPP_FLAGS) main.cpp ini.cpp -o $(TOOLDIR)/$(BINARY)

clean:
	rm -f $(TOOLDIR)/$(BINARY)
