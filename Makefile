# C Project Makefile

#========= Debug mode setup ========#
# Comment one line only!
# -DDEBUG: Debug testing
# -DVERBOSE: Print error messages
# -DTRACE: Trace malloc and free

# Uncomment this line for release
DEBUG := -DNDEBUG -UDEBUG -DVERBOSE -UTRACE

# Uncomment this line for debug
#DEBUG := -DDEBUG -DVERBOSE -DTRACE

#===== Compiler / linker setup =====#
CC := gcc
CFLAGS := -g -O3 -Wall -Wpedantic -Wextra -std=gnu99
DFLAGS := -MP -MMD
LFLAGS := -g -lm
INCLUDE := 
LIBRARY := 

#======== Source code setup ========#
# Directory for all project files and
# the main.c file.
INCLUDE_DIR := ./src
INCLUDE += -I$(INCLUDE_DIR)
SRC_DIR := ./src

# Source files
# CFILES excluses MAIN
CFILES := $(wildcard $(SRC_DIR)/*.c)
HFILES := $(wildcard $(INCLUDE_DIR)/*.h)
IFILES := $(wildcard $(SRC_DIR)/*.inc)

# Important files
MAKEFILE := Makefile
IMPORTANT := $(MAKEFILE) README.md

#=========== Build setup ===========#
# Directory for built files.
BUILD_DIR := build
OFILES := $(CFILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DFILES := $(OFILES:%.o=%.d)

# Tar file of the entire project
TAR := ./project.tar.gz

#============ Main files ===========#
# Standalone text executable sources
# to link with the rest of the code.
MAIN_DIR := main
MCFILES := $(wildcard $(MAIN_DIR)/*.c)
MOFILES := $(MCFILES:$(MAIN_DIR)/%.c=$(BUILD_DIR)/$(MAIN_DIR)/%.o)
MDFILES := $(MOFILES:%.o=%.d)

ALL_EXECUTABLES := $(MCFILES:$(MAIN_DIR)/%.c=%.exe)
TESTS := $(filter test_%.exe,$(ALL_EXECUTABLES))
EXECUTABLES := $(filter-out test_%.exe,$(ALL_EXECUTABLES))

#========== Documentation ==========#
# Doxygen documentation setup
DOC_DIR := doc
WEB_DIR := web
DOXYFILE := Doxyfile
DOXFILES := $(wildcard doc/*.dox)
DOXFILES += $(wildcard doc/*.html)
IMPORTANT += $(DOXYFILE)

#============== Rules ==============#
# Default: just make executables
.PHONY: default
default: $(BUILD_DIR) $(OFILES) $(EXECUTABLES)

# Make just the tests
.PHONY: tests
tests: $(BUILD_DIR) $(OFILES) $(TESTS)

# Default - make the executable
.PHONY: all
all: default tests

# Put all the .o files in the build directory
$(BUILD_DIR):
	-mkdir $@
	-mkdir $@/$(MAIN_DIR)

# Compile the source files
.SECONDARY: $(DFILES)
.SECONDARY: $(OFILES)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(MAKEFILE)
	$(CC) $(CFLAGS) $(DFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@
	
# Compile the main method executables
.SECONDARY: $(MDFILES)
.SECONDARY: $(MOFILES)
$(BUILD_DIR)/$(MAIN_DIR)/%.o: $(MAIN_DIR)/%.c $(MAKEFILE)
	$(CC) $(CFLAGS) $(DFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

# Automatic dependency files
-include $(DFILES)
-include $(MDFILES)

# Documentation
.PHONY: documentation
documentation: $(WEB_DIR)
$(WEB_DIR): $(DOXFILES) $(CFILES) $(HFILES) $(MCFILES)
	doxygen Doxyfile

# Make executable for each driver
%.exe: $(BUILD_DIR)/$(MAIN_DIR)/%.o $(OFILES) 
	$(CC) -o $@ $^ $(LIBRARY) $(LFLAGS)

#============== Clean ==============#
# Clean up build files and executable
.PHONY: clean
clean:
	-rm -rf $(BUILD_DIR) $(WEB_DIR) $(EXECUTABLES) $(TESTS)
	
#============= Archive =============#
# Package all the files into a tar.
.PHONY: tar
tar: $(TAR)
$(TAR): $(CFILES) $(HFILES) $(DOXFILES) $(IMPORTANT) $(LIB_DIR)
	tar -czvf $@ $^

#===================================#
