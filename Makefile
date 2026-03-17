CC = cc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -Werror
TARGET = cmcwatch

ARGS ?= 

BUILD ?= debug

LIBS_RAW = 
LIBS = $(patsubst %,-l%, $(LIBS_RAW))

# Define source directory
SRC_DIR = src
BLD_DIR = build
INC_DIRS_RAW = include
TEST_DIR = tests

INC_DIRS = $(patsubst %,-I%, $(INC_DIRS_RAW))
CFLAGS += $(INC_DIRS)
LDFLAGS =

ifeq ($(BUILD),release)
	CFLAGS += -O3
else
	CFLAGS += -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer
	LDFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
endif

LDFLAGS += -Llib -lzds

SRCS = $(shell find $(SRC_DIR) -name "*.c")
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o, $(SRCS))
TESTS = $(shell find $(TEST_DIR) -name "*.c")

# Compile all right now
all: $(OBJS)
	@$(CC) $(LIBS) $^ $(LDFLAGS) -o $(TARGET)

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c | $(BLD_DIR)
	@echo "Compiling file: $< -> $@"
	@if [ ! -d $(@D) ]; then \
		echo "Creating directory: $(@D)"; \
		mkdir -p $(@D); \
	fi
	@$(CC) $(CFLAGS) $< -c -o $@

$(BLD_DIR):
	@echo "Creating '$@' directory"
	@mkdir $(BLD_DIR)

run:
	@echo "Running $(TARGET)"
	@./$(TARGET) $(ARGS)

test:
	$(CC) $(TESTS)/$(wildcard *.c)

clean:
	@rm -rf $(BLD_DIR) $(TARGET)
