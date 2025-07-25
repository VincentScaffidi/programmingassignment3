CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Target executable names
TARGETS = DistanceVector

# Default target - builds all executables
all: $(TARGETS)

# Build DistanceVector executable from source
DistanceVector: DistanceVector.cpp
	$(CXX) $(CXXFLAGS) -o DistanceVector DistanceVector.cpp

# Clean build artifacts
clean:
	rm -f $(TARGETS)

# Rebuild everything from scratch
rebuild: clean all

# Mark targets that don't create files
.PHONY: all clean rebuild