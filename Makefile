.PHONY: default runtime runtime_dbg release glad run debug clean

GL_VERSION := 4.6

default: runtime

runtime: glad clean_build
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
	cmake --build build

runtime_dbg: glad clean_build
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
	cmake --build build

release: glad clean_build
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
	cmake --build build

glad:
	@glad --api gl:core=$(GL_VERSION) --out-path glad c --loader

run: runtime
	./build/Vivianite

debug: runtime_dbg
	gdb ./build/Vivianite

clean:
	rm -rf build/ glad/ dist/

clean_build:
	rm -rf build/ dist/
