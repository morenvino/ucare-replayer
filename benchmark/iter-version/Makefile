# morenvino@uchicago.edu

# Build options
CC      = gcc
CFLAGS  = -Wall
LDFLAGS = 

# Workspace
SOURCES = $(wildcard *.c)
OBJECTS = $(notdir $(SOURCES:.c=.o))
EXEC1   = directio.out

# Default
all: build test

# Build
build: $(EXEC1)

# Exec
$(EXEC1): $(OBJECTS) 
	$(CC) $(LDFLAGS) $^ -o $@

# Dependencies
#-include $(OBJECTS:.o=.d)

# Generate .object and .dep
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

#$(CC) -MM $(CFLAGS) $< > $*.d

# Clean
clean:
	rm -rf *.o *.d $(EXEC1) 

# Copy
scp: $(EXEC1)
	scpq ./$(EXEC1)

# Test
test: scp
	sshq "sudo ./$(EXEC1) /dev/sdb 8192000"
