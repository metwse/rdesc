# rdesc/tests/Makefile - Robust & Gcovr Compatible

DIST_DIR = ../dist/tests
LIB_TEST = ../dist/test/librdesc.a
LIB_RELEASE = ../dist/release/librdesc.a

CFLAGS_COMMON = -std=c99 -Wall -Wextra -pedantic -Iinclude
TEST_CFLAGS = $(CFLAGS_COMMON) -O0 -g3 --coverage
BENCH_CFLAGS = $(CFLAGS_COMMON) -O3 -g3
FUZZ_CFLAGS = $(CFLAGS_COMMON) -O2 -g3 -DAGRESSIVE_FUZZ

INTEGRATION_SRCS = $(wildcard integration/*.c)
UNIT_SRCS = $(wildcard unit/*.c)
FUZZ_SRCS = $(wildcard fuzz/*.c)

TEST_TARGETS = \
	$(patsubst integration/%.c, $(DIST_DIR)/%.integration.test, $(INTEGRATION_SRCS)) \
	$(patsubst fuzz/%.c, $(DIST_DIR)/%.fuzz.test, $(FUZZ_SRCS)) \
	$(patsubst unit/%.c, $(DIST_DIR)/%.unit.test, $(UNIT_SRCS))

FUZZ_RELEASE_TARGETS = $(patsubst fuzz/%.c, $(DIST_DIR)/%.fuzz.release, $(FUZZ_SRCS))
BENCH_TARGET = $(DIST_DIR)/benchmark

all: directories $(TEST_TARGETS) $(FUZZ_RELEASE_TARGETS) $(BENCH_TARGET)

directories:
	mkdir -p $(DIST_DIR)

# Integration Tests
$(DIST_DIR)/%.integration.test: integration/%.c $(LIB_TEST)
	cd ..; $(CC) $(TEST_CFLAGS) tests/$< tests/$(LIB_TEST) -o tests/$@

# Fuzz Tests (Debug)
$(DIST_DIR)/%.fuzz.test: fuzz/%.c $(LIB_TEST)
	cd ..; $(CC) $(TEST_CFLAGS) tests/$< tests/$(LIB_TEST) -o tests/$@

# Fuzz Tests (Release)
$(DIST_DIR)/%.fuzz.release: fuzz/%.c $(LIB_RELEASE)
	cd ..; $(CC) $(FUZZ_CFLAGS) tests/$< tests/$(LIB_RELEASE) -o tests/$@

# Unit Tests (Sadece kaynak)
$(DIST_DIR)/%.unit.test: unit/%.c
	cd ..; $(CC) $(TEST_CFLAGS) tests/$< -o tests/$@

# Benchmark
$(BENCH_TARGET): benchmark.c $(LIB_RELEASE)
	cd ..; $(CC) $(BENCH_CFLAGS) tests/benchmark.c tests/$(LIB_RELEASE) -o tests/$@

clean:
	rm -rf $(DIST_DIR)

.PHONY: all directories clean
