## Simple build for Preparation/preparation.cpp
## Usage:
##   make        # build
##   make run    # build + run
##   make clean  # remove build artifacts

CXX ?= c++
CXXFLAGS ?=
CXXFLAGS += -std=c++17 -O2 -Wall -Wextra -Wpedantic

SRC := preparation.cpp common/GLShader.cpp
BIN_DIR := bin
BUILD_DIR := build
TARGET := $(BIN_DIR)/preparation
OBJ := $(BUILD_DIR)/preparation.o $(BUILD_DIR)/GLShader.o

CPPFLAGS += -I. -Icommon
CPPFLAGS += $(shell pkg-config --cflags glfw3 glew 2>/dev/null)

LDFLAGS += $(shell pkg-config --libs glfw3 glew 2>/dev/null)
LDFLAGS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(BUILD_DIR)/preparation.o: $(SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c preparation.cpp -o $@

$(BUILD_DIR)/GLShader.o: common/GLShader.cpp common/GLShader.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c common/GLShader.cpp -o $@

-include $(BUILD_DIR)/preparation.d
-include $(BUILD_DIR)/GLShader.d

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

