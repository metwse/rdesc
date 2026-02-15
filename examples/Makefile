DIST_DIR = ../dist/examples
OBJ_DIR = $(DIST_DIR)/obj

LIB = ../dist/test/librdesc.a

# No need to change rules below this line.

include ../config.mk

CFLAGS = $(CFLAGS_COMMON) -O0 -g3 --coverage

SRCS = $(wildcard *.c)

TARGETS = $(patsubst %.c, $(DIST_DIR)/%, $(SRCS))

OBJS = $(wildcard $(DIST_DIR)/*.o)

default: all


.SECONDARY:
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	cd ..; $(CC) $(CFLAGS) -c examples/$< -o examples/$@
	$(CC) -MM $< -MF $(@:.o=.d) -MT $@
$(DIST_DIR)/%: $(OBJ_DIR)/%.o $(LIB) \
		| $(DIST_DIR) check-lib
	$(CC) $(CFLAGS) $^ -o $@

check-lib:
	$(MAKE) -C .. FEATURES='full' MODE='test' -j


# Check library rebuilds with root Makefile.
all: check-lib
	$(MAKE) $(TARGETS)


$(DIST_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) $(DIST_DIR)


-include $(OBJS:.o=.d)

.PHONY: all clean check-lib
