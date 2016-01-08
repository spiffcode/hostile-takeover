BINARY=wis

ifeq ($(REL),1)
CPP_FLAGS=-O2 -DRELEASE_LOGGING -Wno-write-strings -I.. -fsigned-char
OUTDIR=release
REL=1
else
CPP_FLAGS=-g -O0 -DDEBUG_LOGGING -DDEBUG -DDEV_BUILD -Wno-write-strings -I.. -fsigned-char
OUTDIR=debug
REL=0
endif

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
SOCKETSERVER=selectserver
CPP_FLAGS:=$(CPP_FLAGS) -DOSX
CPUS=1
endif
ifeq ($(UNAME), Linux)
SOCKETSERVER=epollserver
CPP_FLAGS:=$(CPP_FLAGS) -DLINUX
CPUS=$(shell grep processor /proc/cpuinfo | wc -l)
endif

ifneq ($(DOCKER),0)
MAKE_ALL = $(shell docker/config AUTH_PREFIX) docker -- run -t --rm -v "$(shell dirname "$(shell pwd)")":/wi -w /wi -u wi $(shell docker/config REGISTRY_PREFIX)/wi_build:$(shell docker/image_version wi_build) /bin/bash -c "cd server && make REL=$(REL) DOCKER=1 -j $(CPUS) d_all"
INSIDE_CHECK := /bin/bash -c "if [ ! -f /.dockerinit ]; then echo 'ERROR: Outside container.' && exit 1; fi"
OUTSIDE_CHECK := /bin/bash -c "if [ -f /.dockerinit ]; then echo 'ERROR: Inside container.' && exit 1; fi"
else
MAKE_ALL = make REL=$(REL) DOCKER=0 -j $(CPUS) d_all
INSIDE_CHECK :=
OUTSIDE_CHECK :=
endif

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

all: outside
	$(MAKE_ALL)

clean:
	rm -f $(OUTDIR)/*

d_all: inside $(OUTDIR)/$(BINARY)
	cp $(OUTDIR)/$(BINARY) docker/$(OUTDIR)

$(OUTDIR)/$(BINARY): $(ALLFILES:%=$(OUTDIR)/%.o) 
	g++ -o $@ $^ -lpthread
	rm -f docker/$(OUTDIR)/$(BINARY)

$(MAINFILES:%=$(OUTDIR)/%.o): $(MAINFILES:%=%.cpp)
	g++ $(CPP_FLAGS) -c $(<D)/$(*F).cpp -o $@

$(BASEFILES:%=$(OUTDIR)/%.o): $(BASEFILES:%=../base/%.cpp)
	g++ $(CPP_FLAGS) -c $(<D)/$(*F).cpp -o $@

$(MPSHAREDFILES:%=$(OUTDIR)/%.o): $(MPSHAREDFILES:%=../mpshared/%.cpp)
	g++ $(CPP_FLAGS) -c $(<D)/$(*F).cpp -o $@

$(INCFILES:%=$(OUTDIR)/%.o): $(INCFILES:%=../inc/%.cpp)
	g++ $(CPP_FLAGS) -c $(<D)/$(*F).cpp -o $@

$(YAJLFILESCPP:%=$(OUTDIR)/%.o): $(YAJLFILESCPP:%=../yajl/wrapper/%.cpp)
	g++ $(CPP_FLAGS) -c $(<D)/$(*F).cpp -o $@

$(YAJLFILESC:%=$(OUTDIR)/%.o): $(YAJLFILESC:%=../yajl/src/%.c)
	g++ $(CPP_FLAGS) -x c -c $(<D)/$(*F).c -o $@

new_base_build_images: outside
	docker/build_image wi_base
	docker/build_image wi_build

push_base_build_images: outside
	docker/push_image wi_base
	docker/push_image wi_build

new_server_image: outside
	$(MAKE) REL=1 clean all
	docker/build_image wi_server

push_server_image: outside
	docker/push_image wi_server

create_gcloud_server: outside
	docker/create_gcloud_server

drain_delete_gcloud_server: outside
	docker/drain_delete_gcloud_server

outside:
	@$(OUTSIDE_CHECK)

inside:
	@$(INSIDE_CHECK)
