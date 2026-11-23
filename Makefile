.PHONY: default runtime runtime_dbg release windows_release glew_windows all_release debug clean
MINGW_PREFIX := /ucrt64
GLEW_DIR := $(CURDIR)/build-windows/glew
GLEW_SOURCE_DIR := $(CURDIR)/build-windows/glew-source
GLEW_URL := https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz

GL_VERSION := 4.6

default: runtime

runtime: glad
	cmake -B build -G Ninja
	cmake --build build

runtime_dbg: glad clean
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
	cmake --build build

release: glad clean_bin
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
	cmake --build build

glew_windows:
	rm -rf $(GLEW_SOURCE_DIR) $(GLEW_DIR)
	mkdir -p build-windows
	curl -L --fail --retry 3 -o build-windows/glew.tar.gz $(GLEW_URL)
	tar -xzf build-windows/glew.tar.gz -C build-windows
	mv build-windows/glew-2.2.0 $(GLEW_SOURCE_DIR)
	make -C $(GLEW_SOURCE_DIR) SYSTEM=linux-mingw64 -j2
	make -C $(GLEW_SOURCE_DIR) SYSTEM=linux-mingw64 \
		GLEW_DEST=$(GLEW_DIR) GLEW_PREFIX=$(GLEW_DIR) install
	ln -sfn $(CURDIR)/external/freetype/include $(GLEW_DIR)/include/freetype2

windows_release: glad glew_windows
	cmake -B build-windows -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64.cmake \
		-DENABLE_LTO=OFF \
		-DBUILD_EDITOR=ON \
		-DCMAKE_C_FLAGS="-DGLEW_STATIC -I$(GLEW_DIR)/include -I$(CURDIR)/external/freetype/include" \
		-DCMAKE_CXX_FLAGS="-DGLEW_STATIC -I$(GLEW_DIR)/include -I$(CURDIR)/external/freetype/include" \
		-DCMAKE_EXE_LINKER_FLAGS="-L$(GLEW_DIR)/lib -lglew32"
	cmake --build build-windows
	cp /usr/x86_64-w64-mingw32/bin/libstdc++-6.dll build-windows/
	cp /usr/x86_64-w64-mingw32/bin/libgcc_s_seh-1.dll build-windows/
	cp /usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll build-windows/
	cp external/openal/soft_oal.dll build-windows/

all_release: release windows_release

glad: $(GLAD_FILES)

$(GLAD_FILES):
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
