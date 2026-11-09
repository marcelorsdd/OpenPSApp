PS5_HOST ?= ps5
PS5_PORT ?= 9021
PROSPERO_PORT ?= 7070
PS5_PAYLOAD_SDK ?= $(HOME)/opt/ps5-payload-sdk

LLVM_CONFIG ?= $(shell brew --prefix llvm 2>/dev/null)/bin/llvm-config
export LLVM_CONFIG

VERSION := $(shell git describe --always --dirty 2>/dev/null || echo dev)

PS5_CXX := $(PS5_PAYLOAD_SDK)/bin/prospero-clang++
PS5_CC := $(PS5_PAYLOAD_SDK)/bin/prospero-clang
HOST_CXX ?= c++
HOST_CC ?= cc

BUILD := build

CORE_SRCS := $(shell find src -name '*.cpp' -not -path 'src/platform/*' -not -name main.cpp | sort)
TEST_SRCS := $(wildcard tests/host/*.cpp)

GEN := $(BUILD)/gen
UI_DIST := ../ui/dist
TILE_PKG := ../tile/dist/playtrack-tile.pkg
TILE_PARAM := ../tile/sce_sys/param.json
GEN_SRCS := $(GEN)/web_assets.cpp $(GEN)/tile_pkg.cpp

INCLUDES := -Isrc -Ithird_party/sqlite -Ithird_party/cjson

CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wframe-larger-than=8192 -Werror=frame-larger-than -MMD -MP $(INCLUDES) \
            -DPLAYTRACK_VERSION=\"$(VERSION)\"
CFLAGS_C := -O2 -w -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION=1 -DSQLITE_DQS=0
THIRD_PARTY_SRCS := third_party/sqlite/sqlite3.c third_party/cjson/cJSON.c

PS5_ELF := $(BUILD)/playtrack.elf
PS5_OBJS := $(patsubst %.cpp,$(BUILD)/ps5/%.o,$(CORE_SRCS) $(GEN_SRCS) src/main.cpp src/platform/ps5.cpp) \
            $(patsubst %.c,$(BUILD)/ps5/%.o,$(THIRD_PARTY_SRCS))
PS5_LIBS := -lSceUserService -lSceAppInstUtil -lkernel_sys

HOST_BIN := $(BUILD)/host/playtrack
HOST_TEST_BIN := $(BUILD)/host/playtrack-tests
HOST_LIB_OBJS := $(patsubst %.cpp,$(BUILD)/host/%.o,$(CORE_SRCS) $(GEN_SRCS) src/platform/host.cpp) \
                 $(patsubst %.c,$(BUILD)/host/%.o,$(THIRD_PARTY_SRCS))
HOST_TEST_OBJS := $(patsubst %.cpp,$(BUILD)/host/%.o,$(TEST_SRCS))

FIXTURE_ROOT ?= $(if $(wildcard tests/fixtures/real/root),tests/fixtures/real/root,tests/fixtures/synthetic/root)

.PHONY: all deploy prospero host-test host-run ui tile clean

all: $(PS5_ELF)

$(GEN)/web_assets.cpp: tools/embed.py $(wildcard $(UI_DIST)/index.html $(UI_DIST)/assets/*)
	python3 tools/embed.py web $(UI_DIST) $@

$(GEN)/tile_pkg.cpp: tools/embed.py $(TILE_PARAM) $(wildcard $(TILE_PKG))
	python3 tools/embed.py tile $(TILE_PKG) $(TILE_PARAM) $@

ui:
	npm --prefix ../ui run build

tile:
	../tile/build.sh

$(PS5_ELF): $(PS5_OBJS)
	$(PS5_CXX) -o $@ $^ $(PS5_LIBS)

$(BUILD)/ps5/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(PS5_CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/ps5/%.o: %.c
	@mkdir -p $(dir $@)
	$(PS5_CC) $(CFLAGS_C) -c -o $@ $<

deploy: $(PS5_ELF)
	$(PS5_PAYLOAD_SDK)/bin/prospero-deploy -h $(PS5_HOST) -p $(PS5_PORT) $<

prospero: $(PS5_ELF)
	curl -fsS -X POST -H 'Content-Type: application/octet-stream' --data-binary @$< \
		"http://$(PS5_HOST):$(PROSPERO_PORT)/api/payloads/upload?name=playtrack.elf"
	@echo
	curl -fsS -X POST -H 'Content-Type: application/json' -d '{"name":"playtrack.elf"}' \
		"http://$(PS5_HOST):$(PROSPERO_PORT)/api/payloads/launch"
	@echo

$(BUILD)/host/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(HOST_CXX) $(CXXFLAGS) -g -c -o $@ $<

$(BUILD)/host/%.o: %.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(CFLAGS_C) -c -o $@ $<

$(HOST_BIN): $(HOST_LIB_OBJS) $(BUILD)/host/src/main.o
	$(HOST_CXX) -o $@ $^ -lpthread

$(HOST_TEST_BIN): $(HOST_LIB_OBJS) $(HOST_TEST_OBJS)
	$(HOST_CXX) -o $@ $^ -lpthread

host-test: $(HOST_TEST_BIN)
	$(HOST_TEST_BIN)

tests/fixtures/synthetic/root: $(HOST_TEST_BIN)
	$(HOST_TEST_BIN) --make-fixture $@

host-run: $(HOST_BIN) $(if $(filter tests/fixtures/synthetic/root,$(FIXTURE_ROOT)),tests/fixtures/synthetic/root)
	PLAYTRACK_ROOT=$(abspath $(FIXTURE_ROOT)) PLAYTRACK_USER=$${PLAYTRACK_USER:-1eb71bbd} $(HOST_BIN)

clean:
	rm -rf $(BUILD) tests/fixtures/synthetic

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
