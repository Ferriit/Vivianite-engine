GL_VERSION := 4.6

CXX := g++
CXX_GLAD := -Iglad/include glad/src/gl.c
CXX_GLFW := $(shell pkg-config --cflags --libs glfw3) -ldl
CXX_FLAGS := -Wall -Wextra -Iglad/include
WARN_FLAGS := -Wunused-but-set-variable -Wno-unused-parameter -Wno-missing-field-initializers

CXX_SRC := $(wildcard src/runtime/*.cpp)
CXX_OBJ := $(patsubst src/runtime/%.cpp, build/%.o, $(CXX_SRC))

DEBUG_OBJ := $(patsubst src/runtime/%.cpp, build/debug/%.o, $(CXX_SRC))

GLAD_FILES := glad/include/glad/gl.h glad/src/gl.c

TOTAL := $(words $(CXX_OBJ))
INDEX = $(shell i=1; for f in $(CXX_SRC); do [ "$$f" = "$1" ] && echo $$i && exit; i=$$((i+1)); done)

.PHONY: default runtime runtime_dbg glad run debug clean

default: runtime

runtime: CXX_FLAGS += $(WARN_FLAGS)
runtime: glad dist/Vivianite

dist/Vivianite: $(CXX_OBJ)
	@mkdir -p dist/
	@printf "\nLinking runtime\n"
	@$(CXX) $^ $(CXX_GLAD) $(CXX_GLFW) -o $@

build/%.o: src/runtime/%.cpp
	@mkdir -p build/
	@printf "[%s / %s] Compiling %s to %s\n" $(call INDEX,$<) $(TOTAL) $< $@
	@$(CXX) $(CXX_FLAGS) -c $< -o $@

glad: $(GLAD_FILES)

$(GLAD_FILES):
	@glad --api gl:core=$(GL_VERSION) --out-path glad c --loader

runtime_dbg: glad dist/Vivianite_dbg

build/debug/%.o: src/runtime/%.cpp
	@mkdir -p build/debug/
	@$(CXX) $(CXX_FLAGS) -DDBG -O0 -g -c $< -o $@

dist/Vivianite_dbg: $(DEBUG_OBJ)
	@mkdir -p dist/
	@printf "\nLinking debug runtime\n"
	@$(CXX) $^ $(CXX_GLAD) $(CXX_GLFW) -o $@

run: runtime
	./dist/Vivianite

debug: runtime_dbg
	gdb ./dist/Vivianite_dbg

clean:
	rm -rf build/ glad/ dist/
