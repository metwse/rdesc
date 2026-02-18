FUZZ_DIR = fuzz
INTEGRATION_DIR = integration
UNIT_DIR = unit

DIST_DIR = ../dist/tests
OBJ_DIR = $(DIST_DIR)/obj

LIB_RELEASE = ../dist/release/librdesc.a
LIB_TEST = ../dist/test/librdesc.a

# No need to change rules below this line.

include ../config.mk

FUZZ_CFLAGS = $(CFLAGS_COMMON) -O2 -g3 -DAGRESSIVE_FUZZ
TEST_CFLAGS = $(CFLAGS_COMMON) -O0 -g3 --coverage

FUZZ_SRCS = $(wildcard $(FUZZ_DIR)/*.c)
INTEGRATION_SRCS = $(wildcard $(INTEGRATION_DIR)/*.c)
UNIT_SRCS = $(wildcard $(UNIT_DIR)/*.c)

TEST_TARGETS = \
	$(patsubst $(INTEGRATION_DIR)/%.c, $(DIST_DIR)/%.integration.test, $(INTEGRATION_SRCS)) \
	$(patsubst $(FUZZ_DIR)/%.c, $(DIST_DIR)/%.fuzz.test, $(FUZZ_SRCS)) \
	$(patsubst $(UNIT_DIR)/%.c, $(DIST_DIR)/%.unit.test, $(UNIT_SRCS))

FUZZ_TARGETS = \
	$(patsubst $(FUZZ_DIR)/%.c, $(DIST_DIR)/%.fuzz.release, $(FUZZ_SRCS))

OBJS = $(wildcard $(OBJ_DIR)/*.o)


# Check library rebuilds with root Makefile.
default: check-lib-test
	$(MAKE) $(TEST_TARGETS)

fuzz: check-lib-release
	$(MAKE) $(FUZZ_TARGETS)

all: default fuzz


# Test build rules compile from project root for gcovr compatibility, And then
# dependency files are regenerated as compilation from project root broke
# dependency tracking.

# Integration and fuzz tests links the entire library, unit tests uses include
# directives and only include source files needed.

# - INTEGRATION ---------------------------------------------------------------
.SECONDARY:
$(OBJ_DIR)/%.integration.test.o: $(INTEGRATION_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_CFLAGS) -c tests/$< -o tests/$@
	$(CC) -MM $< -MF $(@:.o=.d) -MT $@
$(DIST_DIR)/%.integration.test: $(OBJ_DIR)/%.integration.test.o $(LIB_TEST) \
		| $(DIST_DIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

# - FUZZ ----------------------------------------------------------------------
.SECONDARY:
$(OBJ_DIR)/%.fuzz.release.o: $(FUZZ_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(FUZZ_CFLAGS) -c tests/$< -o tests/$@
	$(CC) -MM $< -MF $(@:.o=.d) -MT $@
$(DIST_DIR)/%.fuzz.release: $(OBJ_DIR)/%.fuzz.release.o $(LIB_RELEASE) \
		| $(DIST_DIR)
	$(CC) $(FUZZ_CFLAGS) $^ -o $@

.SECONDARY:
$(OBJ_DIR)/%.fuzz.test.o: $(FUZZ_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_CFLAGS) -c tests/$< -o tests/$@
	$(CC) -MM $< -MF $(@:.o=.d) -MT $@
$(DIST_DIR)/%.fuzz.test: $(OBJ_DIR)/%.fuzz.test.o $(LIB_TEST) \
		| $(DIST_DIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

# - UNIT ----------------------------------------------------------------------
.SECONDARY:
$(OBJ_DIR)/%.unit.test.o: $(UNIT_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_CFLAGS) -c tests/$< -o tests/$@
	$(CC) -MM $< -MF $(@:.o=.d) -MT $@
$(DIST_DIR)/%.unit.test: $(OBJ_DIR)/%.unit.test.o | $(DIST_DIR)
	$(CC) $(TEST_CFLAGS) $< -o $@


# Library build rules.
check-lib-test:
	$(MAKE) -C .. FEATURES='full' MODE='test' -j

check-lib-release:
	$(MAKE) -C .. FEATURES='full' MODE='release' -j


$(DIST_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) $(DIST_DIR)


-include $(OBJS:.o=.d)

.PHONY: default all clean fuzz \
	check-lib-test check-lib-release
