#
#  Makefile
#
#  Created by Hilton Lipschitz on 2015-09-01.
#  Copyright (c) 2015 Maritime Capital LP. All rights reserved.
#  More info at https://hiltmon.com/blog/2015/09/28/the-simple-c-plus-plus-makefile-executable-edition/
#  Useful info about makefiles at https://makefiletutorial.com/

# HIL: No spaces or comments after otherwise it captures them!

# Determine the platform
UNAME_S := $(shell uname -s)
HOST := $(shell hostname)

# CXX
ifeq ($(UNAME_S),Darwin)
  CXX := clang++ -arch x86_64
else
  CXX := g++
endif

# Folders
SRCDIR := src
BUILDDIR := build
TARGETDIR := bin

# Final Paths 
INSTALLBINDIR := /usr/local/bin

# Code Lists
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

# Folder Lists
# Note: Intentionally excludes the root of the include folder so the lists are clean
INCDIRS := $(shell find $(SRCDIR)/**/* -name '*.hpp' -exec dirname {} \; | sort | uniq)
INCLIST := $(patsubst $(SRCDIR)/%,-I $(SRCDIR)/%,$(INCDIRS))
BUILDLIST := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(INCDIRS))

# folder with additional includes
INCLOCALFLAGS := -I /usr/local -I /usr/local/include
ifneq (,$(findstring arc,$(HOST)))
    INCLOCALFLAGS += -I$(HOME)
endif

# folder with boost headers (necessary for geometry routines)
BOOSTPATH := /opt/homebrew/Cellar/boost/1.80.0/include

# folder with fmt header (necessary for formatting routines)
FMTPATH := ./external_include/fmt-master/include/fmt

# Shared Compiler Flags
CXXFLAGS := -c
INC := -I $(SRCDIR) $(INCLIST) $(INCLOCALFLAGS) -I $(BOOSTPATH) -I $(FMTPATH)
LIB := -L /usr/local/lib

# $(info 'including'$(INC))

# Platform Specific Compiler Flags
ifeq ($(UNAME_S),Linux)
    CXXFLAGS += -std=gnu++14 -Ofast # -fPIC

    # PostgreSQL Special
    #PG_VER := 9.3
    #INC += -I /usr/pgsql-$(PG_VER)/include
    #LIB += -L /usr/pgsql-$(PG_VER)/lib
else
  CXXFLAGS += -std=c++1y -stdlib=libc++ -Ofast
endif

$(TARGETDIR)/CBMsimulator: singleStrainSim/CBMsimulator.o $(OBJECTS)
	@mkdir -p $(TARGETDIR)
	@echo "Linking..."
	@echo "Linking $(TARGETDIR)/CBMsimulator"; $(CXX) $^ -o $(TARGETDIR)/CBMsimulator $(LIB)

singleStrainSim/CBMsimulator.o: singleStrainSim/main.$(SRCEXT)
	@echo "Compiling $<..."; $(CXX) $(CXXFLAGS) $(INC) -I singleStrainSim -c -o $@ $<
	
$(TARGETDIR)/viralMixing: viralMixing/viralMixing.o $(OBJECTS)
	@mkdir -p $(TARGETDIR)
	@echo "Linking..."
	@echo "Linking $(TARGETDIR)/viralMixing"; $(CXX) $^ -o $(TARGETDIR)/viralMixing $(LIB)

viralMixing/viralMixing.o: viralMixing/viralMixing.$(SRCEXT)
	@echo "Compiling $<..."; $(CXX) $(CXXFLAGS) $(INC) -I viralMixing -c -o $@ $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDLIST)
	@echo "Compiling $<..."; $(CXX) $(CXXFLAGS) $(INC) -c -o $@ $<

clean:
	@echo "Cleaning..."; $(RM) -r $(BUILDDIR) $(TARGETDIR) singleStrainSim/CBMsimulator.o viralMixing/viralMixing.o

.PHONY: clean

