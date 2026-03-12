FUZZ_DIR = fuzz
INTEGRATION_DIR = integration
UNIT_DIR = unit

DIST_DIR = ../dist/tests
OBJ_DIR = $(DIST_DIR)/obj

# No need to change rules below this line.

CFLAGS_COMMON = -std=c99 -Wall -Wextra -pedantic

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


default: $(TEST_TARGETS) $(FUZZ_TARGETS)


RDESC_DIR := ..
RDESC_FEATURES := full

RDESC_MODE := test
include ../rdesc.mk
LIB_TEST := $(RDESC)

RDESC_MODE := release
include ../rdesc.mk
LIB_RELEASE := $(RDESC)


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

$(DIST_DIR)/%.fuzz.test: $(OBJ_DIR)/%.fuzz.test.o $(LIB_TEST) | $(DIST_DIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

# - UNIT ----------------------------------------------------------------------
.SECONDARY:
$(OBJ_DIR)/%.unit.test.o: $(UNIT_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_CFLAGS) -c tests/$< -o tests/$@
	$(CC) -MM $< -MF $(@:.o=.d) -MT $@
$(DIST_DIR)/%.unit.test: $(OBJ_DIR)/%.unit.test.o | $(DIST_DIR)
	$(CC) $(TEST_CFLAGS) $< -o $@


$(DIST_DIR) $(OBJ_DIR):
	mkdir -p $@


-include $(OBJS:.o=.d)

.PHONY: default all fuzz
