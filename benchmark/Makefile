# morenvino@uchicago.edu

# Workspace
CSOURCES   = $(wildcard *.c)
COBJECTS   = $(notdir $(CSOURCES:.c=.o))

CXXSOURCES = $(wildcard *.cpp)
CXXOBJECTS = $(notdir $(CXXSOURCES:.cpp=.o))

EXEC1   = benchmark.out

# Build options
CC        = gcc
CFLAGS    = -pthread -Wall

CXX       = g++
CXXFLAGS  = -std=c++11 -pthread -Wall

LDFLAGS   = -std=c++11 -pthread -Wl,--no-as-needed -lpthread

# Default
all: build test

# Build
build: $(EXEC1)

# Exec
$(EXEC1): $(COBJECTS) $(CXXOBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@
#$(CC)  $(LDFLAGS) $^ -o $@

# Dependencies
-include $(COBJECTS:.o=.d)
-include $(CXXOBJECTS:.o=.d)

# Generate .object and .dep
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -MM  $< > $*.d

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(CXX) $(CXXFLAGS) -MM $< > $*.d

# Clean
clean:
	rm -rf *.o *.d $(EXEC1) 

# Copy
scp: $(EXEC1)
	scpq ./config.ini
	scpq ./$(EXEC1)

# Test
test: scp
	sshq "sudo ./$(EXEC1)"
	
#sshq "sudo ./$(EXEC1) /dev/sdb 8192"
#sshq "sudo ./$(EXEC1) /dev/sdb 8192000"
