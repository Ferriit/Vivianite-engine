GL_VERSION := 4.6

CXX := g++

CXX_GLAD := -Iglad/include glad/src/gl.c
CXX_GLFW := $(shell pkg-config --cflags --libs glfw3) -ldl
CXX_FLAGS := -o build/Vivianite

CXX_SRC := src/engine/engine.cpp

.PHONY: default glad engine run clean

default: engine

engine: glad
	mkdir -p build/
	$(CXX) $(CXX_SRC) $(CXX_GLAD) $(CXX_GLFW) $(CXX_FLAGS)

engine_dbg: glad
	mkdir -p build/
	$(CXX) $(CXX_SRC) $(CXX_GLAD) $(CXX_GLFW) $(CXX_FLAGS) -DDEBUG -O0 -g

glad:
	glad --api gl:core=$(GL_VERSION) --out-path glad c --loader

run: engine
	./build/Vivianite

debug: engine_dbg
	gdb ./build/Vivianite

clean:
	rm -rf build/ glad/
