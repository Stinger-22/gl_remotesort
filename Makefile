CC = gcc
CXX = g++
CXXFLAGS = -Wall -std=c++17 -I./src/include

SOURCEDIR = ./src/
INCLUDEDIR = ./src/include/
BUILDDIR  = ./build/

all: server client
	@echo "Build successful!"

server: $(BUILDDIR)server.o $(BUILDDIR)util.o
	@echo "Building: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build finished: $@"

client: $(BUILDDIR)client.o $(BUILDDIR)util.o
	@echo "Building: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build finished: $@"

$(BUILDDIR)client.o: $(SOURCEDIR)client.cpp
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)server.o: $(SOURCEDIR)server.cpp
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)util.o: $(SOURCEDIR)util.cpp $(INCLUDEDIR)util.hpp
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	-rm $(BUILDDIR)*.o