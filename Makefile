#####################################
######## CSci 5607 Makefile #########
#####################################

#===== Compiler / linker setup =====#
CC := gcc
DFLAGS := -MP -MMD
CFLAGS := -s -O3 -Wall
LFLAGS := -s
INCLUDE := 
LIBRARY := 

#======== Source code setup ========#
# Source code files
SRC_DIR := src
INCLUDE += -I$(SRC_DIR)
CFILES := $(wildcard $(SRC_DIR)/*.c)

#=========== Build setup ===========#
# Executable
EXECUTABLE_DIR := .
EXECUTABLE := $(EXECUTABLE_DIR)/build.exe

# Build files
BUILD_DIR := build
OFILES := $(CFILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DFILES := $(OFILES:.o=.d)

# Documentation files
DOC_DIR := doc

#============== Rules ==============#
# Default - make the executable
.PHONY: all
all: $(BUILD_DIR) $(EXECUTABLE)

# Put all the .o files in the build directory
$(BUILD_DIR):
	mkdir $@

# Generate auto-dependency files (.d) instead
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $< -o $@

# Build executable
$(EXECUTABLE): $(OFILES)
	$(CC) $(OFILES) $(LIBRARY) $(LFLAGS) -o $@

# Doxygen documentation
$(DOC_DIR):
	doxygen Doxyfile
	
# Auto-generated dependencies
-include $(DFILES)
	
# Clean up build files and executable
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE)

# Clean up absolutely everything
.PHONY: spotless
spotless: clean
	rm -rf $(DOC_DIR)
