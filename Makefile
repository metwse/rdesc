CC=gcc
RM=rm -rf

CFLAGS_COMMON = -std=c99 -Wall -Wextra -fPIC -MMD -MP

CFLAGS = $(CFLAGS_COMMON) -O2
TFLAGS = $(CFLAGS_COMMON) -O0 -g3 --coverage

SRC_DIR=src
INC_DIR=include
TEST_DIR=tests
DIST_DIR=dist
OBJ_DIR=$(DIST_DIR)/obj
DEBUG_OBJ_DIR=$(DIST_DIR)/debug_obj


# no need to change rules below this line
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.c, $(DEBUG_OBJ_DIR)/%.o, $(SRCS))

TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, $(DEBUG_OBJ_DIR)/%.test.o, $(TEST_SRCS))
TEST_TARGETS = $(patsubst $(TEST_DIR)/%.c, $(DIST_DIR)/%.test, $(TEST_SRCS))


all: $(DIST_DIR)/librdesc.so
tests: $(TEST_TARGETS)

# release library link
$(DIST_DIR)/librdesc.so: $(OBJS) | $(DIST_DIR)
	$(CC) $(CFLAGS) -shared -o $@ $^

# test binaries
$(DIST_DIR)/%.test: $(DEBUG_OBJ_DIR)/%.test.o $(DEBUG_OBJS) | $(DIST_DIR)
	$(CC) $(TFLAGS) -o $@ $^


# Object rules
# release
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# debug
$(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_OBJ_DIR)
	$(CC) $(TFLAGS) -c $< -o $@

# test
$(DEBUG_OBJ_DIR)/%.test.o: $(TEST_DIR)/%.c | $(DEBUG_OBJ_DIR)
	$(CC) $(TFLAGS) -c $< -o $@


$(DIST_DIR) $(OBJ_DIR) $(DEBUG_OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) $(DIST_DIR) docs

docs:
	doxygen


.SECONDARY: $(OBJS) $(DEBUG_OBJS) $(TEST_OBJS)
-include $(OBJS:.o=.d)
-include $(DEBUG_OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)

.PHONY: all clean docs tests
