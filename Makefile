CXX = clang++

# Detect SDL2 path automatically
SDL2_PREFIX := $(shell brew --prefix sdl2 2>/dev/null || echo "/opt/homebrew")
SDL2_TTF_PREFIX := $(shell brew --prefix sdl2_ttf 2>/dev/null || echo "/opt/homebrew")

CXXFLAGS = -std=c++11 -Wall -I$(SDL2_PREFIX)/include/SDL2 -I$(SDL2_TTF_PREFIX)/include/SDL2
LDFLAGS = -L$(SDL2_PREFIX)/lib -L$(SDL2_TTF_PREFIX)/lib -lSDL2 -lSDL2_ttf

TARGET = space_invaders
SOURCE = space_invaders.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	@echo "Using SDL2 from: $(SDL2_PREFIX)"
	@echo "Using SDL2_ttf from: $(SDL2_TTF_PREFIX)"
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET) $(LDFLAGS)
	@echo "✓ Build successful! Run with: ./$(TARGET)"

clean:
	rm -f $(TARGET)
	@echo "✓ Cleaned"

run: $(TARGET)
	./$(TARGET)

# Check dependencies
check-deps:
	@echo "Checking dependencies..."
	@brew list sdl2 > /dev/null 2>&1 && echo "✓ SDL2 is installed" || echo "✗ SDL2 not found. Install with: brew install sdl2"
	@brew list sdl2_ttf > /dev/null 2>&1 && echo "✓ SDL2_ttf is installed" || echo "✗ SDL2_ttf not found. Install with: brew install sdl2_ttf"

.PHONY: all clean run check-deps
