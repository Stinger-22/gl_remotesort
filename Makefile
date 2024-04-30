CC = gcc
CXX = g++
CXXFLAGS = -Wall -std=c++17 -I./src/include

SOURCEDIR = ./src/
INCLUDEDIR = ./src/include/
BUILDDIR  = ./build/

all: server client
	@echo "Build successful!"

server: $(BUILDDIR)appServer.o $(BUILDDIR)util.o $(BUILDDIR)server.o
	@echo "Building: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build finished: $@"

client: $(BUILDDIR)appClient.o $(BUILDDIR)util.o $(BUILDDIR)client.o
	@echo "Building: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build finished: $@"

$(BUILDDIR)appClient.o: $(SOURCEDIR)appClient.cpp | $(BUILDDIR)
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)appServer.o: $(SOURCEDIR)appServer.cpp | $(BUILDDIR)
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)server.o: $(SOURCEDIR)server.cpp $(INCLUDEDIR)server.hpp | $(BUILDDIR)
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)client.o: $(SOURCEDIR)client.cpp $(INCLUDEDIR)client.hpp | $(BUILDDIR)
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)socket.o: $(SOURCEDIR)socket.cpp $(INCLUDEDIR)socket.hpp | $(BUILDDIR)
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)util.o: $(SOURCEDIR)util.cpp $(INCLUDEDIR)util.hpp | $(BUILDDIR)
	@echo "Compiling: $@"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir $(BUILDDIR)

.PHONY: clean

clean:
	-rm $(BUILDDIR)*.o