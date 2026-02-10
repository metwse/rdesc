FEATURES ?= stack

# List of source files linked to the library. You can use your own stack
# implementation after removing stack element below.
LIB = cfg $(FEATURES)

CC = gcc
RM = rm -rf

CFLAGS_COMMON = -std=c99 -Wall -Wextra -pedantic -fPIC -MMD -MP

CFLAGS = $(CFLAGS_COMMON) -O2
TFLAGS = $(CFLAGS_COMMON) -O0 -g3 --coverage

SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
DIST_DIR = dist
EXAMPLE_DIR = examples

OBJ_DIR = $(DIST_DIR)/obj/release
DEBUG_OBJ_DIR = $(DIST_DIR)/obj/debug
TEST_OBJ_DIR = $(DIST_DIR)/obj/test
EXAMPLE_OBJ_DIR = $(DIST_DIR)/obj/example


# no need to change rules below this line
SRCS = $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
EXAMPLE_SRCS = $(wildcard $(EXAMPLE_DIR)/*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.c, $(DEBUG_OBJ_DIR)/%.o, $(SRCS))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, $(TEST_OBJ_DIR)/%.o, $(TEST_SRCS))
EXAMPLE_OBJS = $(patsubst $(EXAMPLE_DIR)/%.c, $(EXAMPLE_OBJ_DIR)/%.o, $(EXAMPLE_SRCS))

TEST_TARGETS = $(patsubst $(TEST_DIR)/%.c, $(DIST_DIR)/%.test, $(TEST_SRCS))
EXAMPLE_TARGETS = $(patsubst $(EXAMPLE_DIR)/%.c, $(DIST_DIR)/%, $(EXAMPLE_SRCS))


default: $(DIST_DIR)/librdesc.so $(DIST_DIR)/librdesc.a
all: default examples tests
tests: $(TEST_TARGETS)
examples: $(EXAMPLE_TARGETS)

PREFIX ?= /usr/local
install: $(DIST_DIR)/librdesc.so $(DIST_DIR)/librdesc.a
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/rdesc/

	install -m 755 $(DIST_DIR)/librdesc.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(DIST_DIR)/librdesc.a $(DESTDIR)$(PREFIX)/lib/

	install -m 644 include/* $(DESTDIR)$(PREFIX)/include/rdesc/

# release static library
$(DIST_DIR)/librdesc.a: $(foreach o,$(LIB),$(OBJ_DIR)/$o.o) | $(DIST_DIR)
	ar rcs $@ $^

# release shared library
$(DIST_DIR)/librdesc.so: $(foreach o,$(LIB),$(OBJ_DIR)/$o.o) | $(DIST_DIR)
	$(CC) $(CFLAGS) -shared -o $@ $^

# test binaries
$(DIST_DIR)/%.test: $(TEST_OBJ_DIR)/%.o $(DEBUG_OBJS) | $(DIST_DIR)
	$(CC) $(TFLAGS) -o $@ $^

# example binaries
$(DIST_DIR)/%: $(EXAMPLE_OBJ_DIR)/%.o $(DEBUG_OBJS) | $(DIST_DIR)
	$(CC) $(TFLAGS) -o $@ $^


# Object rules
# release
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# debug
$(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_OBJ_DIR)
	$(CC) $(TFLAGS) -c $< -o $@

# test
$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(TFLAGS) -c $< -o $@

# example
$(EXAMPLE_OBJ_DIR)/%.o: $(EXAMPLE_DIR)/%.c | $(EXAMPLE_OBJ_DIR)
	$(CC) $(TFLAGS) -c $< -o $@


$(DIST_DIR) $(OBJ_DIR) $(DEBUG_OBJ_DIR) $(TEST_OBJ_DIR) $(EXAMPLE_OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) $(DIST_DIR) docs

docs:
	doxygen


.SECONDARY: $(OBJS) $(DEBUG_OBJS) $(TEST_OBJS)
-include $(OBJS:.o=.d)
-include $(DEBUG_OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)
-include $(EXAMPLE_OBJS:.o=.d)

.PHONY: default all clean docs tests examples install
