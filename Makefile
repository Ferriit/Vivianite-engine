.PHONY: default runtime runtime_dbg release debug clean

GL_VERSION := 4.6

default: runtime

runtime: glad clean
	cmake -B build -G Ninja
	cmake --build build

runtime_dbg: glad clean
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
	cmake --build build

release: glad clean
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
	cmake --build build

glad: $(GLAD_FILES)

$(GLAD_FILES):
	@glad --api gl:core=$(GL_VERSION) --out-path external/glad c --loader

run: runtime
	./build/Vivianite

debug: runtime_dbg
	gdb ./build/Vivianite

clean:
	rm -rf build/ dist/
