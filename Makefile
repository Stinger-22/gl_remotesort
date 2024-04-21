CC = gcc
CXX = g++
CXXFLAGS = -Wall -std=c++17

SOURCEDIR = ./src/
INCLUDEDIR = ./src/include/
BUILDDIR  = ./build/

all: server client
	@echo "Build successful!"

server: $(BUILDDIR)server.o
	@echo "Building: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build finished: $@"

client: $(BUILDDIR)client.o
	@echo "Building: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build finished: $@"

$(BUILDDIR)client.o: $(SOURCEDIR)client.cpp
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)server.o: $(SOURCEDIR)server.cpp
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	-rm $(BUILDDIR)*.o