# morenvino@uchicago.edu

# Workspace
CSOURCES   = $(wildcard *.c)
COBJECTS   = $(notdir $(CSOURCES:.c=.o))

CXXSOURCES = $(wildcard *.cpp)
CXXOBJECTS = $(notdir $(CXXSOURCES:.cpp=.o))

EXEC1   = main.out

# Build options
CC        = gcc
CFLAGS    = -pthread -Wall

CXX       = g++
CXXFLAGS  = -std=c++11 -pthread -Wall -O2

LDFLAGS   = -std=c++11 -pthread -Wl,--no-as-needed -lpthread -lrt -static-libstdc++

# Default
all: build

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

# Scp to Qemu
scp: $(EXEC1)
	scpq ../timing -r

# Test
test:
	sudo ./$(EXEC1) RAD-BE-10000.trace
	
#sshq "sudo ./$(EXEC1) /dev/sdb 8192"
#sshq "sudo ./$(EXEC1) /dev/sdb 8192000"
