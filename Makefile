#####################################
######## CSci 5607 Makefile #########
#####################################

#===== Compiler / linker setup =====#
CC := gcc
DFLAGS := -MP -MMD
CFLAGS := -s -O3 -Wall -DDEBUG -DTRACE
LFLAGS := -s -lm
INCLUDE := 
LIBRARY := 

#======== Source code setup ========#
# Source code files
SRC_DIR := src
INCLUDE += -I$(SRC_DIR)
CFILES := $(wildcard $(SRC_DIR)/*.c)

#=========== Build setup ===========#
# Build files
BUILD_DIR := build
OFILES := $(CFILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DFILES := $(OFILES:.o=.d)

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
	echo $(EXECUTABLES)

.PHONY: test
test: $(BUILD_DIR) $(DRIVER_BUILD_DIR) $(EXECUTABLES)

# Put all the .o files in the build directory
$(BUILD_DIR):
	-mkdir $@

# Also create subdirectory for driver files
$(DRIVER_BUILD_DIR): $(BUILD_DIR)
	-mkdir $@

# Generate auto-dependency files (.d) instead
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $< -o $@

# Generate driver files also
$(DRIVER_BUILD_DIR)/%.o: $(DRIVER_DIR)/%.c $(DRIVER_BUILD_DIR)
	$(CC) $(CFLAGS) $(DFLAGS) $(INCLUDE) -c $< -o $@

# Make executable for each driver
%.exe: $(DRIVER_BUILD_DIR)/%.o $(OFILES)
	$(CC) $(OFILES) $< $(LIBRARY) $(LFLAGS) -o $@

# Doxygen documentation
$(DOC_DIR):
	doxygen Doxyfile

#============= Include =============#
# Auto-generated dependencies
-include $(DFILES)
-include $(DRIVER_DFILES)

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

$(TURNIN):
	tar -cvf $@ README.md Makefile Doxyfile src/*.c src/*.h src/driver/main.c
