#
#
#

PROJECT=capture

SRCS= main.cpp \
      BlockMap.cpp \
      CaptureProcessor.cpp

CXX=g++
CPPFLAGS= -Wall -O2 -g -std=c++11
LDFLAGS=

DIST_FILES= Makefile src/* capture.stp *.py *.sh

BUILD=build

#
#
#

srcname=src/$(strip $(1))
objname=$(BUILD)/objs/$(basename $(notdir $(strip $(1)))).o
objpaths=$(foreach name,$(1),$(call objname,$(name)))
OBJS=$(call objpaths,$(SRCS))

define compile_cxx

$(call objname,$(1)): $(call srcname,$(1)) | $(BUILD)/objs
	$(CXX) -Isrc $(CPPFLAGS) $(OPTS) $$< -c -o $$@

endef

define compile_sources

$(foreach name,$(1),$(call compile_cxx,$(name)))

endef

.PHONY: all dist clean

all: $(PROJECT)

$(BUILD)/objs:
	@mkdir -p $@

$(eval $(call compile_sources,$(SRCS)))

$(PROJECT): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

dist:
	tar -cvzf $(PROJECT).tar.gz $(DIST_FILES)

clean:
	rm -f $(PROJECT)
	rm -f $(PROJECT).tar.gz
	rm -rf $(BUILD)
