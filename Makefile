CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -Iinclude -Isrc

LDFLAGS = -lglfw -ldl -lGL

SRC = src/engine/engine.cpp src/glad.c

OBJ = $(SRC:src/%.cpp=build/%.o)
OBJ := $(OBJ:src/%.c=build/%.o)

TARGET = engine

GLAD_DIR = include
GLAD_VERSION = 4.6
GLAD_PROFILE = core


all: glad $(TARGET)


$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)


build/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


build/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


glad:
	@echo "Generating GLAD..."
	glad --api gl:$(GLAD_VERSION) --profile $(GLAD_PROFILE) \
	     --out-path $(GLAD_DIR) c


clean:
	rm -rf build $(TARGET)

.PHONY: all clean glad
