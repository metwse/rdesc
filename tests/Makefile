FUZZ_DIR = fuzz
INTEGRATION_DIR = integration
UNIT_DIR = unit

DIST_DIR = ../dist/tests
OBJ_DIR = $(DIST_DIR)/obj

LIB_DEBUG = ../dist/debug/librdesc.a
LIB_RELEASE =  ../dist/release/librdesc.a
LIB_TEST = ../dist/test/librdesc.a

FUZZ_CFLAGS = $(CFLAGS_COMMON) -O2 -g3 -fsanitize=address -DAGRESSIVE_FUZZ
TEST_CFLAGS = $(CFLAGS_COMMON) -O0 -g3 -fsanitize=address
TEST_COVR_CFLAGS = $(CFLAGS_COMMON) -O0 -g3 --coverage

# No need to change rules below this line.

include ../config.mk

FUZZ_SRCS = $(wildcard $(FUZZ_DIR)/*.c)
INTEGRATION_SRCS = $(wildcard $(INTEGRATION_DIR)/*.c)
UNIT_SRCS = $(wildcard $(UNIT_DIR)/*.c)

COVR_TARGETS = \
	$(patsubst $(INTEGRATION_DIR)/%.c, $(DIST_DIR)/%.integration.covr, $(INTEGRATION_SRCS)) \
	$(patsubst $(FUZZ_DIR)/%.c, $(DIST_DIR)/%.fuzz.covr, $(FUZZ_SRCS)) \
	$(patsubst $(UNIT_DIR)/%.c, $(DIST_DIR)/%.unit.covr, $(UNIT_SRCS))

DEBUG_TARGETS = \
	$(patsubst $(INTEGRATION_DIR)/%.c, $(DIST_DIR)/%.integration.debug, $(INTEGRATION_SRCS)) \
	$(patsubst $(UNIT_DIR)/%.c, $(DIST_DIR)/%.unit.debug, $(UNIT_SRCS))

FUZZ_TARGETS = \
	$(patsubst $(FUZZ_DIR)/%.c, $(DIST_DIR)/%.fuzz.release, $(FUZZ_SRCS))

all: covr debug fuzz

# Test build rules. Compile from project root for gcovr compatibility.
.SECONDARY:
$(OBJ_DIR)/%.integration.debug.o: $(INTEGRATION_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_CFLAGS) -c tests/$(INTEGRATION_DIR)/$*.c -o tests/$@
$(DIST_DIR)/%.integration.debug: $(OBJ_DIR)/%.integration.debug.o $(LIB_DEBUG) | $(DIST_DIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

.SECONDARY:
$(OBJ_DIR)/%.integration.covr.o: $(INTEGRATION_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_COVR_CFLAGS) -c tests/$(INTEGRATION_DIR)/$*.c -o tests/$@
$(DIST_DIR)/%.integration.covr: $(OBJ_DIR)/%.integration.covr.o $(LIB_TEST) | $(DIST_DIR)
	$(CC) $(TEST_COVR_CFLAGS) $^ -o $@

.SECONDARY:
$(OBJ_DIR)/%.fuzz.release.o: $(FUZZ_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(FUZZ_CFLAGS) -c tests/$(FUZZ_DIR)/$*.c -o tests/$@
$(DIST_DIR)/%.fuzz.release: $(OBJ_DIR)/%.fuzz.release.o $(LIB_RELEASE) | $(DIST_DIR)
	$(CC) $(FUZZ_CFLAGS) $^ -o $@

.SECONDARY:
$(OBJ_DIR)/%.fuzz.covr.o: $(FUZZ_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_COVR_CFLAGS) -c tests/$(FUZZ_DIR)/$*.c -o tests/$@
$(DIST_DIR)/%.fuzz.covr: $(OBJ_DIR)/%.fuzz.covr.o $(LIB_TEST) | $(DIST_DIR)
	$(CC) $(TEST_COVR_CFLAGS) $^ -o $@

.SECONDARY:
$(OBJ_DIR)/%.unit.debug.o: $(UNIT_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_CFLAGS) -c tests/$(UNIT_DIR)/$*.c -o tests/$@
$(DIST_DIR)/%.unit.debug: $(OBJ_DIR)/%.unit.debug.o | $(DIST_DIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

.SECONDARY:
$(OBJ_DIR)/%.unit.covr.o: $(UNIT_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(TEST_COVR_CFLAGS) -c tests/$(UNIT_DIR)/$*.c -o tests/$@
$(DIST_DIR)/%.unit.covr: $(OBJ_DIR)/%.unit.covr.o | $(DIST_DIR)
	$(CC) $(TEST_COVR_CFLAGS) $^ -o $@

# Library build rules.
$(LIB_DEBUG):
	$(MAKE) -C .. FEATURES='full' MODE='debug' -j

$(LIB_TEST):
	$(MAKE) -C .. FEATURES='full' MODE='test' -j

$(LIB_RELEASE):
	$(MAKE) -C .. FEATURES='full' MODE='release' -j

$(DIST_DIR) $(OBJ_DIR):
	mkdir -p $@


clean:
	$(RM) $(DIST_DIR)


covr: $(COVR_TARGETS)

debug: $(DEBUG_TARGETS)

fuzz: $(FUZZ_TARGETS)


.PHONY: all covr debug fuzz clean
