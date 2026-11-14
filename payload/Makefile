PS5_HOST ?= ps5
PS5_PORT ?= 9021
PROSPERO_PORT ?= 7070
PS5_PAYLOAD_SDK ?= $(HOME)/opt/ps5-payload-sdk
PS5_PAYLOAD_SDK_VERSION := v0.45
PS5_PAYLOAD_SDK_SHA256 := 5fac64093d0e3e69f2584ce7935f24f703f622d9681f1495c85b9f00d1291a2c
PS5_PAYLOAD_SDK_URL := https://github.com/ps5-payload-dev/sdk/releases/download/$(PS5_PAYLOAD_SDK_VERSION)/ps5-payload-sdk.zip
PS5_PAYLOAD_SDK_STAMP := $(PS5_PAYLOAD_SDK)/.openpsapp-sdk-version

LLVM_CONFIG_CANDIDATES := $(shell command -v brew >/dev/null 2>&1 && echo "$$(brew --prefix llvm)/bin/llvm-config") \
                          $(foreach v,23 22 21 20 19 18,llvm-config-$(v)) llvm-config
ifeq ($(origin LLVM_CONFIG),undefined)
LLVM_CONFIG := $(shell for c in $(LLVM_CONFIG_CANDIDATES); do command -v "$$c" && break; done 2>/dev/null)
else
LLVM_CONFIG := $(or $(shell command -v "$(LLVM_CONFIG)" 2>/dev/null),$(LLVM_CONFIG))
endif
ifneq ($(LLVM_CONFIG),)
export LLVM_CONFIG
endif

DEV_CA ?=
VERSION := $(shell git describe --tags --always --dirty 2>/dev/null || echo dev)$(if $(DEV_CA),+devca)

PS5_CXX := $(PS5_PAYLOAD_SDK)/bin/prospero-clang++
PS5_CC := $(PS5_PAYLOAD_SDK)/bin/prospero-clang
HOST_CXX ?= c++
HOST_CC ?= cc

BUILD := $(if $(DEV_CA),build-dev,build)

CORE_SRCS := $(shell find src -name '*.cpp' -not -path 'src/platform/*' -not -name main.cpp | sort)
TEST_SRCS := $(wildcard tests/host/*.cpp)

GEN := $(BUILD)/gen
UI_DIST := ../ui/dist
TILE_PKG := ../tile/dist/openpsapp-tile.pkg
TILE_PARAM := ../tile/sce_sys/param.json
GEN_SRCS := $(GEN)/web_assets.cpp $(GEN)/tile_pkg.cpp $(if $(DEV_CA),$(GEN)/dev_ca.cpp)

MBEDTLS := -Ithird_party/mbedtls/include -DMBEDTLS_CONFIG_FILE='"net/mbedtls_config.h"'
INCLUDES := -Isrc -I$(GEN) -Ithird_party/sqlite -Ithird_party/cjson $(MBEDTLS)

CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -Wframe-larger-than=8192 -Werror=frame-larger-than -MMD -MP $(INCLUDES) $(if $(DEV_CA),-DOPENPSAPP_DEV_CA)
CFLAGS_C := -O2 -w -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION=1 -DSQLITE_DQS=0 -Isrc $(MBEDTLS)
THIRD_PARTY_SRCS := third_party/sqlite/sqlite3.c third_party/cjson/cJSON.c $(sort $(wildcard third_party/mbedtls/library/*.c))

PS5_ELF := $(BUILD)/openpsapp.elf
PS5_OBJS := $(patsubst %.cpp,$(BUILD)/ps5/%.o,$(CORE_SRCS) $(GEN_SRCS) src/main.cpp src/platform/ps5.cpp) \
            $(patsubst %.c,$(BUILD)/ps5/%.o,$(THIRD_PARTY_SRCS))
PS5_LIBS := -lSceUserService -lSceSystemService -lSceAppInstUtil -lSceRegMgr -lkernel_sys

HOST_BIN := $(BUILD)/host/openpsapp
HOST_TEST_BIN := $(BUILD)/host/openpsapp-tests
HOST_LIB_OBJS := $(patsubst %.cpp,$(BUILD)/host/%.o,$(CORE_SRCS) $(GEN_SRCS) src/platform/host.cpp) \
                 $(patsubst %.c,$(BUILD)/host/%.o,$(THIRD_PARTY_SRCS))
HOST_TEST_OBJS := $(patsubst %.cpp,$(BUILD)/host/%.o,$(TEST_SRCS))

FIXTURE_ROOT ?= $(if $(wildcard tests/fixtures/real/root),tests/fixtures/real/root,tests/fixtures/synthetic/root)

.PHONY: all deploy prospero host-test host-run ui tile clean FORCE

all: $(PS5_ELF)

$(GEN)/web_assets.cpp: tools/embed.py $(wildcard $(UI_DIST)/index.html $(UI_DIST)/assets/*)
	python3 tools/embed.py web $(UI_DIST) $@

$(GEN)/tile_pkg.cpp: tools/embed.py $(TILE_PARAM) $(wildcard $(TILE_PKG))
	python3 tools/embed.py tile $(TILE_PKG) $(TILE_PARAM) $@

$(GEN)/dev_ca.cpp: tools/embed.py $(DEV_CA)
	python3 tools/embed.py devca $(DEV_CA) $@

$(GEN)/version.hpp: FORCE
	@mkdir -p $(GEN)
	@printf '#pragma once\n#define OPENPSAPP_VERSION "%s"\n' '$(VERSION)' > $@.tmp
	@cmp -s $@.tmp $@ && rm -f $@.tmp || mv $@.tmp $@

$(BUILD)/ps5/src/main.o $(BUILD)/host/src/main.o: $(GEN)/version.hpp

ui:
	npm --prefix .. ci
	npm --prefix .. run build -w @openpsapp/ui

tile:
	../tile/build.sh

.PHONY: sdk sdk-check

sdk:
	@test ! -e '$(PS5_PAYLOAD_SDK)' || { echo "sdk: $(PS5_PAYLOAD_SDK) already exists; remove it or set PS5_PAYLOAD_SDK" >&2; exit 1; }
	rm -rf $(BUILD)/sdk && mkdir -p $(BUILD)/sdk
	curl -fsSL -o $(BUILD)/sdk/ps5-payload-sdk.zip $(PS5_PAYLOAD_SDK_URL)
	echo '$(PS5_PAYLOAD_SDK_SHA256)  $(BUILD)/sdk/ps5-payload-sdk.zip' | shasum -a 256 -c -
	unzip -q $(BUILD)/sdk/ps5-payload-sdk.zip -d $(BUILD)/sdk
	mkdir -p '$(dir $(PS5_PAYLOAD_SDK))'
	mv $(BUILD)/sdk/ps5-payload-sdk '$(PS5_PAYLOAD_SDK)'
	echo '$(PS5_PAYLOAD_SDK_VERSION)' > '$(PS5_PAYLOAD_SDK_STAMP)'
	rm -rf $(BUILD)/sdk

sdk-check:
	@test -x '$(PS5_CXX)' || { echo "ps5-payload-sdk not found at $(PS5_PAYLOAD_SDK); run 'make sdk' to install $(PS5_PAYLOAD_SDK_VERSION) or set PS5_PAYLOAD_SDK" >&2; exit 1; }
	@test ! -f '$(PS5_PAYLOAD_SDK_STAMP)' || test "$$(cat '$(PS5_PAYLOAD_SDK_STAMP)')" = '$(PS5_PAYLOAD_SDK_VERSION)' || \
		{ echo "ps5-payload-sdk at $(PS5_PAYLOAD_SDK) is $$(cat '$(PS5_PAYLOAD_SDK_STAMP)') but this payload needs $(PS5_PAYLOAD_SDK_VERSION); run 'make sdk PS5_PAYLOAD_SDK=<new dir>'" >&2; exit 1; }

$(PS5_ELF): $(PS5_OBJS) | sdk-check
	$(PS5_CXX) -o $@ $^ $(PS5_LIBS)

$(BUILD)/ps5/%.o: %.cpp | sdk-check
	@mkdir -p $(dir $@)
	$(PS5_CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/ps5/%.o: %.c | sdk-check
	@mkdir -p $(dir $@)
	$(PS5_CC) $(CFLAGS_C) -c -o $@ $<

deploy: $(PS5_ELF)
	$(PS5_PAYLOAD_SDK)/bin/prospero-deploy -h $(PS5_HOST) -p $(PS5_PORT) $<

prospero: $(PS5_ELF)
	curl -fsS -X POST -H 'Content-Type: application/octet-stream' --data-binary @$< \
		"http://$(PS5_HOST):$(PROSPERO_PORT)/api/payloads/upload?name=openpsapp.elf"
	@echo
	curl -fsS -X POST -H 'Content-Type: application/json' -d '{"name":"openpsapp.elf"}' \
		"http://$(PS5_HOST):$(PROSPERO_PORT)/api/payloads/launch"
	@echo

$(BUILD)/host/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(HOST_CXX) $(CXXFLAGS) -DOPENPSAPP_TLS_TEST_SERVER -g -c -o $@ $<

$(BUILD)/host/%.o: %.c
	@mkdir -p $(dir $@)
	$(HOST_CC) $(CFLAGS_C) -DOPENPSAPP_TLS_TEST_SERVER -c -o $@ $<

$(HOST_BIN): $(HOST_LIB_OBJS) $(BUILD)/host/src/main.o
	$(HOST_CXX) -o $@ $^ -lpthread

$(HOST_TEST_BIN): $(HOST_LIB_OBJS) $(HOST_TEST_OBJS)
	$(HOST_CXX) -o $@ $^ -lpthread

host-test: $(HOST_TEST_BIN)
	$(HOST_TEST_BIN)

tests/fixtures/synthetic/root: $(HOST_TEST_BIN)
	$(HOST_TEST_BIN) --make-fixture $@

host-run: $(HOST_BIN) $(if $(filter tests/fixtures/synthetic/root,$(FIXTURE_ROOT)),tests/fixtures/synthetic/root)
	OPENPSAPP_ROOT=$(abspath $(FIXTURE_ROOT)) OPENPSAPP_USER=$${OPENPSAPP_USER:-1eb71bbd} $(HOST_BIN)

LLVM_BIN := $(shell command -v brew >/dev/null 2>&1 && echo "$$(brew --prefix llvm)/bin")
FUZZ_CXX ?= $(if $(LLVM_BIN),$(LLVM_BIN)/clang++,clang++)
FUZZ_CC ?= $(if $(LLVM_BIN),$(LLVM_BIN)/clang,clang)
FUZZ_DIR := $(BUILD)/fuzz
FUZZ_SECONDS ?= 60
FUZZ_MAX_LEN ?= 65536
FUZZ_SAN := -fsanitize=address,undefined
FUZZ_CXXFLAGS := -std=c++20 -g -O1 -fno-omit-frame-pointer $(INCLUDES) $(FUZZ_SAN)
FUZZ_NAMES := url http_message http_request ucp t2pd npbind timeutil config_json cloud_json
FUZZ_SUPPORT_CXX := src/net/http_message.cpp src/net/url.cpp src/http/router.cpp src/util/json.cpp \
                    src/util/fs.cpp src/util/sha256.cpp src/util/timeutil.cpp src/formats/ucp.cpp \
                    src/formats/t2pd.cpp src/formats/npbind.cpp src/config.cpp src/cloud/store.cpp \
                    tests/fuzz/fuzz_stubs.cpp
FUZZ_SUPPORT_C := third_party/cjson/cJSON.c
FUZZ_SUPPORT_OBJS := $(patsubst %.cpp,$(FUZZ_DIR)/obj/%.o,$(FUZZ_SUPPORT_CXX)) \
                     $(patsubst %.c,$(FUZZ_DIR)/obj/%.o,$(FUZZ_SUPPORT_C))
FUZZ_BINS := $(patsubst %,$(FUZZ_DIR)/fuzz_%,$(FUZZ_NAMES))

$(FUZZ_DIR)/obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(FUZZ_CXX) $(FUZZ_CXXFLAGS) -fsanitize=fuzzer-no-link -c -o $@ $<

$(FUZZ_DIR)/obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(FUZZ_CC) -O1 -g -w -Ithird_party/cjson $(FUZZ_SAN) -fsanitize=fuzzer-no-link -c -o $@ $<

$(FUZZ_DIR)/fuzz_%: tests/fuzz/fuzz_%.cpp $(FUZZ_SUPPORT_OBJS)
	@mkdir -p $(dir $@)
	$(FUZZ_CXX) $(FUZZ_CXXFLAGS) -fsanitize=fuzzer -o $@ $< $(FUZZ_SUPPORT_OBJS)

FUZZ_SEEDS := tests/fuzz/seeds
FUZZ_CORPUS ?= $(FUZZ_DIR)/corpus
FUZZ_ARTIFACTS ?= $(FUZZ_DIR)/artifacts

.PHONY: fuzz fuzz-build fuzz-seeds
fuzz-seeds:
	python3 tests/fuzz/make_seeds.py

fuzz-build: $(FUZZ_BINS)

fuzz: fuzz-build
	@set -e; for name in $(FUZZ_NAMES); do \
		echo "=== fuzzing $$name for $(FUZZ_SECONDS)s ==="; \
		mkdir -p $(FUZZ_CORPUS)/$$name $(FUZZ_ARTIFACTS)/$$name; \
		$(FUZZ_DIR)/fuzz_$$name -max_total_time=$(FUZZ_SECONDS) -max_len=$(FUZZ_MAX_LEN) \
			-artifact_prefix=$(FUZZ_ARTIFACTS)/$$name/ -print_final_stats=1 \
			$(FUZZ_CORPUS)/$$name $(FUZZ_SEEDS)/$$name || exit 1; \
	done

clean:
	rm -rf build build-dev tests/fixtures/synthetic

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
