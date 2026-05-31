CXX := clang++

TARGET := HunNes
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
BIN := $(BUILD_DIR)/$(TARGET)

SRC := $(sort $(wildcard src/*.cpp src/Mapper/*.cpp))
OBJ := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

SDL_PREFIX := $(shell if command -v brew >/dev/null 2>&1; then prefix="$$(brew --prefix sdl2 2>/dev/null)"; [ -d "$$prefix" ] && printf '%s' "$$prefix"; fi)
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 2>/dev/null)
SDL_LIBS := $(shell pkg-config --libs sdl2 2>/dev/null)

# Homebrew does not require pkg-config, so use the formula prefix as a fallback.
ifeq ($(strip $(SDL_CFLAGS)$(SDL_LIBS)),)
ifneq ($(strip $(SDL_PREFIX)),)
SDL_CFLAGS := -I$(SDL_PREFIX)/include
SDL_LIBS := -L$(SDL_PREFIX)/lib -lSDL2
endif
endif

SDL_FOUND := $(if $(strip $(SDL_CFLAGS)$(SDL_LIBS)),yes,no)

CXXFLAGS ?= -O2 -Wall -Wextra -std=c++17
CXXFLAGS += -Iinclude $(SDL_CFLAGS)
LDFLAGS ?=
LDLIBS ?= $(SDL_LIBS)

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
	@echo "Install it with: brew install sdl2" >&2
	@exit 1
endif

$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJ) $(LDFLAGS) -o $@ $(LDLIBS)

# Do not start parallel compilation until the required SDL2 dependency is available.
$(OBJ): | check-sdl

$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEP)

clean:
	rm -rf $(BUILD_DIR)
