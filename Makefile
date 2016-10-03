#####################################
######## CSci 5607 Makefile #########
#####################################

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
CFLAGS := -s -O3 -Wall -std=gnu99
DFLAGS := -MP -MMD
LFLAGS := -s -lm
INCLUDE := 
LIBRARY := 

#======== Source code setup ========#
# Source code files
SRC_DIR := src
INCLUDE += -I$(SRC_DIR)
CFILES := $(wildcard $(SRC_DIR)/*.c)
HFILES := $(wildcard $(SRC_DIR)/*.h)

#=========== Build setup ===========#
# Build files
BUILD_DIR := build
OFILES := $(CFILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DFILES := $(OFILES:%.o=%.d)

# Documentation files
DOC_DIR := doc

# Turnin file
TURNIN := turnin.tar

# Driver files
DRIVER_DIR := $(SRC_DIR)/driver
DRIVER_BUILD_DIR := $(BUILD_DIR)/driver
DRIVER_CFILES := $(wildcard $(DRIVER_DIR)/*.c)
DRIVER_OFILES := $(DRIVER_CFILES:$(DRIVER_DIR)/%.c=$(DRIVER_BUILD_DIR)/%.o)
DRIVER_DFILES := $(DRIVER_OFILES:.o=.d)

# Main program to create
MAIN := ./main.exe

# Non-main executables to create
EXECUTABLES := $(subst $(MAIN),,$(DRIVER_CFILES:$(DRIVER_DIR)/%.c=./%.exe))

#============== Rules ==============#
# Default - make the executable
.PHONY: all
all: $(BUILD_DIR) $(DRIVER_BUILD_DIR) $(MAIN)

.PHONY: test
test: $(BUILD_DIR) $(DRIVER_BUILD_DIR) $(EXECUTABLES)

# Put all the .o files in the build directory
$(BUILD_DIR):
	-mkdir $@

# Also create subdirectory for driver files
$(DRIVER_BUILD_DIR):
	-mkdir $@

# Generate auto-dependency files (.d) instead
.SECONDARY: $(DFILES)
.SECONDARY: $(OFILES)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

-include $(DFILES)

# Generate driver files also
.SECONDARY: $(DRIVER_DFILES)
.SECONDARY: $(DRIVER_OFILES)
$(DRIVER_BUILD_DIR)/%.o: $(DRIVER_DIR)/%.c
	$(CC) $(CFLAGS) $(DFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

-include $(DRIVER_DFILES)

# Make executable for each driver
%.exe: $(DRIVER_BUILD_DIR)/%.o $(OFILES)
	$(CC) $(OFILES) $< $(LIBRARY) $(LFLAGS) -o $@

# Doxygen documentation
$(DOC_DIR):
	doxygen Doxyfile

#============== Clean ==============#
# Clean up build files and executable
.PHONY: clean
clean:
	-rm -rf $(BUILD_DIR) $(EXECUTABLES) $(MAIN) $(TURNIN)

# Clean up absolutely everything
.PHONY: spotless
spotless: clean
	-rm -rf $(DOC_DIR)

#============= Turnin ==============#
.PHONY: turnin
turnin: $(TURNIN)

$(TURNIN): $(CFILES) $(HFILES) README.md Doxyfile Makefile src/driver/main.c
	tar -cvf $@ $^
