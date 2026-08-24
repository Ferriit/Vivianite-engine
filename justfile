default: run_runtime

runtime: glad
	cmake -B build -G Ninja
	cmake --build build

runtime_dbg: glad clean
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
	cmake --build build

release: glad clean_bin
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
	cmake --build build

windows_release: glad
	mkdir -p build-windows
	cmake -B build-windows -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
		-DENABLE_LTO=ON
	cmake --build build-windows
	cp $(MINGW_PREFIX)/bin/libgcc_s_seh-1.dll build-windows/
	cp $(MINGW_PREFIX)/bin/libwinpthread-1.dll build-windows/
	cp $(MINGW_PREFIX)/bin/libstdc++-6.dll build-windows/

all_release: release windows_release

glad:
	@glad --api gl:core=$(GL_VERSION) --out-path external/glad c --loader

run_runtime: runtime
	./build/VivianiteRuntime

run_editor: runtime
	./build/VivianiteEditor

debug: runtime_dbg
	MESA_DEBUG=context gdb ./build/Vivianite

clean_bin:
	rm -f build/Vivianite*
	rm -f build/Debug/Vivianite*
	rm -f build/Release/Vivianite*

clean:
	rm -rf build/ build-windows/ dist/
