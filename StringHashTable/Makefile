# Debug building: make DEBUG=1

# This makefile source dir. It's assumed that a project (or subproject) has its own Makefile 
# located in source files root dir. Thus, we can address source files while building from a
# non-source dir (shadow building).
SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Tools

# To override a tool either use make's command line option (e.g. make CXX=g++-11) or symlink 
# (e.g. ln -s /usr/bin/g++-11 g++) available within search path.

CXX ?= g++  # make's default g++
CXXLINK = $(CXX)
RM_FILE = rm -f

# DEFINES (without -D)

ifdef DEBUG
    DEFINES += DEBUG _DEBUG
else
    DEFINES += NDEBUG
endif

# INCLUDES (without -I)

INCLUDES += \
    $(SRC_DIR)ext/ClickHouse/dbms/src \
    $(SRC_DIR)ext/ClickHouse/contrib/cityhash102/include \
    $(SRC_DIR)ext/ClickHouse/libs/libcommon/include \
	
#$(SRC_DIR)ext/boost-v1.74.0 \

# FLAGS (without DEFINES and INCLUDES)

CXXFLAGS = -std=c++17 -Wmissing-declarations -Wall -Wextra -Wno-format-security
#-mavx2 -mbmi2 -mfma
ifdef DEBUG
    CXXFLAGS += -O0 -g
else
    CXXFLAGS += -O3
endif

# Files

SOURCES = \
    main.cpp \

OBJECTS = $(SOURCES:.cpp=.o)
EXEC = StringHashTable

# Building

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXXLINK) -o $@ $^ $(LDFLAGS)

%.o: $(SRC_DIR)%.cpp
	$(CXX) -c -o $@ $(CPPFLAGS) $(CXXFLAGS) $(addprefix -D,$(DEFINES)) \
	$(addprefix -I,$(INCLUDES)) -MMD $<

# Setup dependency of OBJECTS from header files (via d-files created with -MMD compiler option)
ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),distclean)
else
    # Note: -include $(OBJECTS:%.o=%.d) has form of valid targets, so:
    # 1) don't use leading TAB characters (as recipes);
    # 2) it must be included after the first (default) target, unless .DEFAULT_GOAL is specified.
    -include $(OBJECTS:%.o=%.d)
endif

# Cleaning

clean:
	$(RM_FILE) *.o *.d

distclean: clean
	$(RM_FILE) $(EXEC)

.PHONY: all clean distclean
