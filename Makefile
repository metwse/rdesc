CC=gcc
RM=rm -rf

CFLAGS=-std=c99 -O2 -Wall -Wextra -fPIC
# test flags
TFLAGS=-std=c99 -O0 -g3 -Wall -Wextra --coverage

BIN_DIR=bin
OBJS=rdesc.o stack.o

# no need to change anything below this line
TEST:=$(filter tests bin/%.test, $(MAKECMDGOALS))

ifdef TEST
CFLAGS=$(TFLAGS)
endif


librdesc.so: $(OBJS)
	$(CC) $(CFLAGS)   -shared -o $@ $^


$(BIN_DIR)/%.test: $(OBJS) tests/%.o | $(BIN_DIR)
	$(CC) $(CFLAGS)   -o $@ $^


$(foreach target,$(target),$(shell $(CC) -MM *.c))

.SECONDARY:
$(foreach test_target,$(test_target),$(shell $(CC) -MM tests/*.c))


all: librdesc.so

clean:
	$(RM) librdesc.a librdesc.so \
		$(wildcard *.o) $(wildcard */*.o) \
		$(wildcard *.gcno) $(wildcard */*.gcno) \
		$(wildcard *.gcda) $(wildcard */*.gcda)


.PHONY: all clean


$(BIN_DIR):
	mkdir $@
