#####################################
######## CSci 5607 Makefile #########
#####################################

#===== Compiler / linker setup =====#
CC := gcc
DFLAGS := -MP -MMD
CFLAGS := -s -O3 -Wall -DDEBUG
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
EXECUTABLE := build.exe

# Build files
BUILD_DIR := build
OFILES := $(CFILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DFILES := $(OFILES:.o=.d)

# Documentation files
DOC_DIR := doc

# Turnin file
TURNIN := turnin.tar

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
	rm -rf $(BUILD_DIR) $(EXECUTABLE) $(TURNIN)

# Clean up absolutely everything
.PHONY: spotless
spotless: clean
	rm -rf $(DOC_DIR)

#============= Turnin ==============#
.PHONY: turnin
turnin: $(TURNIN)

$(TURNIN): $(SRC_DIR) Makefile Doxyfile README.md test.txt test.ppm
	tar -cvf $@ $^