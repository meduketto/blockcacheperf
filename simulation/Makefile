#
#
#

PROJECT=blockcachesim

SRCS= main.cpp \
      Cache.cpp \
      Simulator.cpp \
      source/CapturedAccesses.cpp \
      eviction/LRU.cpp \
      eviction/ARC.cpp

UNITTESTS= LruListTest.cpp

CXX=g++
CPPFLAGS=-Wall -O2 -g -std=c++11
LDFLAGS=

DIST_FILES= Makefile src/*

BUILD=build

GTEST_PATH=/usr/src/gtest
GTEST_CPPFLAGS=-I$(GTEST_PATH)
GTEST_LDFLAGS=-L$(GTEST_PATH) -lgtest_main -lgtest -lpthread

#
#
#

srcname=src/$(strip $(1))
unittestname=unittests/$(strip $(1))
objname=$(BUILD)/objs/$(basename $(notdir $(strip $(1)))).o
objpaths=$(foreach name,$(1),$(call objname,$(name)))
unittestobjname=$(BUILD)/unittests/$(basename $(notdir $(strip $(1))))
unittestobjpaths=$(foreach name,$(1),$(call unittestobjname,$(name)))
OBJS=$(call objpaths,$(SRCS))
UNITTESTOBJS=$(call unittestobjpaths,$(UNITTESTS))

define compile_cxx

$(call objname,$(1)): $(call srcname,$(1)) | $(BUILD)/objs
	$(CXX) -MMD -MP -Isrc $(CPPFLAGS) $(OPTS) $$< -c -o $$@

endef

define compile_sources

$(foreach name,$(1),$(call compile_cxx,$(name)))

endef

define compile_unittest

$(call unittestobjname,$(1)): $(call unittestname,$(1)) | $(BUILD)/unittests
	$(CXX) -MMD -MP -Isrc $(GTEST_CPPFLAGS) $(CPPFLAGS) $(LDFLAGS) $(GTEST_LDFLAGS) $(OPTS) $$< -o $$@

endef

define compile_unittests

$(foreach name,$(1),$(call compile_unittest,$(name)))

endef

.PHONY: all test dist clean

all: $(PROJECT)

$(BUILD)/objs $(BUILD)/unittests:
	@mkdir -p $@

$(eval $(call compile_sources,$(SRCS)))

-include $(BUILD)/objs/*.d

$(eval $(call compile_unittests,$(UNITTESTS)))

-include $(BUILD)/unittests/*.d

$(PROJECT): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

test: $(PROJECT) $(UNITTESTOBJS)
	@for cmd in $(UNITTESTOBJS); \
	do \
		$$cmd; \
	done

dist:
	tar -cvzf $(PROJECT).tar.gz $(DIST_FILES)

clean:
	rm -f $(PROJECT)
	rm -f $(PROJECT).tar.gz
	rm -rf $(BUILD)
