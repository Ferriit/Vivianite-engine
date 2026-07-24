GL_VERSION := 4.6

CXX := g++
CXX_GLAD_INCLUDE := -Iglad/include glad/src/gl.c
CXX_GL := -lglfw -ldl
CXX_FLAGS := -o build/Vivianite

CXX_SRC = src/engine/engine.cpp

.PHONY: glad

default: engine

engine: glad
	mkdir -p build/
	$(CXX) $(CXX_SRC) $(CXX_GLAD_INCLUDE) $(CXX_GL) $(CXX_FLAGS)

glad:
	glad --api gl:core=$(GL_VERSION) --out-path glad c --loader
