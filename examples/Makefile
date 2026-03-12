DIST_DIR = ../dist/examples
OBJ_DIR = $(DIST_DIR)/obj

# No need to change rules below this line.

CFLAGS = -std=c99 -Wall -Wextra -pedantic -O0 -g3 --coverage

SRCS = $(wildcard *.c)

TARGETS = $(patsubst %.c, $(DIST_DIR)/%, $(SRCS))

OBJS = $(wildcard $(OBJ_DIR)/*.o)


default: $(TARGETS)


RDESC_DIR := ..
RDESC_FEATURES := full

RDESC_MODE := test
include ../rdesc.mk


.SECONDARY:
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	cd ..; $(CC) $(CFLAGS) -c examples/$< -o examples/$@
	$(CC) -MM $< -MF $(@:.o=.d) -MT $@
$(DIST_DIR)/%: $(OBJ_DIR)/%.o $(RDESC) \
		| $(DIST_DIR)
	$(CC) $(CFLAGS) $^ -o $@


$(DIST_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	$(RM) $(DIST_DIR)


-include $(OBJS:.o=.d)

.PHONY: default
