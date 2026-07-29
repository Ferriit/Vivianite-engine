.PHONY: default runtime runtime_dbg glad run debug clean setup

setup:
	@echo "Run the appropriate dependency installer for your platform."
ifeq ($(OS),Windows_NT)
	@if [ -n "$$MSYSTEM" ]; then \
		echo "MSYS2 detected ($$MSYSTEM)"; \
		pacman -Syu --needed --noconfirm \
			mingw-w64-ucrt-x86_64-gcc \
			mingw-w64-ucrt-x86_64-glfw \
			mingw-w64-ucrt-x86_64-glm \
			mingw-w64-ucrt-x86_64-mesa \
			mingw-w64-ucrt-x86_64-pkgconf \
			mingw-w64-ucrt-x86_64-python \
			mingw-w64-ucrt-x86_64-python-pip; \
		python -m venv venv; \
		venv/Scripts/pip install glad; \
	else \
		echo "Windows detected."; \
		echo "Install MSYS2 first:"; \
		echo "  winget install MSYS2.MSYS2"; \
		echo ""; \
		echo "Then open the MSYS2 UCRT64 terminal and run:"; \
		echo "  make setup"; \
	fi
else
	@if command -v apt >/dev/null; then \
		echo "Debian/Ubuntu detected"; \
		sudo apt update && sudo apt install -y \
			build-essential \
			pkg-config \
			libglfw3-dev \
			libglm-dev \
			libgl1-mesa-dev \
			python3-pip \
			python3.12-venv && \
		python3 -m venv venv && \
		venv/bin/pip install glad; \
	\
	elif command -v pacman >/dev/null; then \
		echo "Arch detected"; \
		sudo pacman -S --needed \
			base-devel \
			pkgconf \
			glfw-x11 \
			glm \
			python \
			python-pip && \
		python -m venv venv && \
		venv/bin/pip install glad; \
	\
	elif command -v dnf >/dev/null; then \
		echo "Fedora detected"; \
		sudo dnf install -y \
			gcc-c++ \
			make \
			pkgconf-pkg-config \
			glfw-devel \
			glm-devel \
			mesa-libGL-devel \
			python3-pip \
			python3-venv && \
		python3 -m venv venv && \
		venv/bin/pip install glad; \
	\
	elif command -v zypper >/dev/null; then \
		echo "openSUSE detected"; \
		sudo zypper install -y \
			gcc-c++ \
			make \
			pkg-config \
			glfw3-devel \
			glm-devel \
			Mesa-libGL-devel \
			python3-pip \
			python3-venv && \
		python3 -m venv venv && \
		venv/bin/pip install glad; \
	\
	else \
		echo "Unsupported Linux distribution"; \
		exit 1; \
	fi
endif

GL_VERSION := 4.6

CXX := g++
CXX_GLAD := -Iglad/include glad/src/gl.c
CXX_GLFW := $(shell pkg-config --cflags --libs glfw3) -ldl
CXX_FLAGS := -Wall -Wextra -Iglad/include
WARN_FLAGS := -Wunused-but-set-variable -Wno-unused-parameter -Wno-missing-field-initializers
RELEASE_FLAGS := -O3 -DNDEBUG -flto

CXX_SRC := $(wildcard src/runtime/*.cpp)
CXX_OBJ := $(patsubst src/runtime/%.cpp, build/%.o, $(CXX_SRC))

DEBUG_OBJ := $(patsubst src/runtime/%.cpp, build/debug/%.o, $(CXX_SRC))

GLAD_FILES := glad/include/glad/gl.h glad/src/gl.c

TOTAL := $(words $(CXX_OBJ))
INDEX = $(shell i=1; for f in $(CXX_SRC); do [ "$$f" = "$1" ] && echo $$i && exit; i=$$((i+1)); done)

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

release: $(CXX_FLAGS) += $(RELEASE_FLAGS)
release: runtime

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
