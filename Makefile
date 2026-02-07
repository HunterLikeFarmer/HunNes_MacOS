CXX ?= g++

TARGET := HunNes
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN := $(BUILD_DIR)/$(TARGET)

SRC := $(sort $(wildcard src/*.cpp src/Mapper/*.cpp))
OBJ := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)
SDL_FOUND := $(if $(strip $(SDL_CFLAGS)$(SDL_LIBS)),yes,no)

CXXFLAGS ?= -O2 -Wall -Wextra -std=c++17
CXXFLAGS += -Iinclude $(SDL_CFLAGS)
LDFLAGS ?=
LDLIBS ?= $(SDL_LIBS)

# Older libstdc++ versions need explicit filesystem linkage.
ifeq ($(findstring g++,$(notdir $(CXX))),g++)
GXX_MAJOR := $(shell $(CXX) -dumpversion | cut -d. -f1)
ifneq ($(shell [ "$(GXX_MAJOR)" -lt 9 ] 2>/dev/null && echo yes),)
LDLIBS += -lstdc++fs
endif
endif

.PHONY: all clean run rebuild check-sdl

all: check-sdl $(BIN)

rebuild: clean all

run: all
	./$(BIN)

check-sdl:
ifeq ($(SDL_FOUND),yes)
	@:
else
	@echo "SDL2 not found." >&2
	@echo "Install libsdl2-dev (Debian/Ubuntu), SDL2-devel (Fedora), or sdl2 (Arch)." >&2
	@exit 1
endif

$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEP)

clean:
	rm -rf $(BUILD_DIR)
