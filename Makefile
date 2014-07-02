# morenvino@uchicago.edu

# Workspace
CSOURCES   = $(wildcard *.c)
COBJECTS   = $(notdir $(CSOURCES:.c=.c.o))

CXXSOURCES = $(wildcard *.cpp)
CXXOBJECTS = $(notdir $(CXXSOURCES:.cpp=.cpp.o))

EXEC1   = directio.out

# Build options
CC        = gcc
CFLAGS    = -pthread -Wall

CXX       = g++
CXXFLAGS  = -std=c++11 -pthread -Wall

LDFLAGS   = -pthread -Wl,--no-as-needed -lpthread

# Default
all: build test

# Build
build: $(EXEC1)

# Exec
$(EXEC1): $(COBJECTS) $(CXXOBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@
#$(CC)  $(LDFLAGS) $^ -o $@

# Dependencies
-include $(COBJECTS:.c.o=.d)
-include $(CXXOBJECTS:.cpp.o=.d)

# Generate .object and .dep
%.c.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC)  -MM  $< > $*.d

%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(CXX) -MM $< > $*.d

# Clean
clean:
	rm -rf *.o *.d $(EXEC1) 

# Copy
scp: $(EXEC1)
	scpq ./$(EXEC1)

# Test
test: scp
	sshq "sudo ./$(EXEC1) /dev/sdb 8192000"
#sshq "sudo ./$(EXEC1) /dev/sdb 8192"
