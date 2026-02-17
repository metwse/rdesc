# Select features from 'stack', 'dump_cst', 'dump_bnf' or use 'full'.
FEATURES ?= stack
FULL_FEATURES = stack dump_cst dump_bnf
# release, debug, or test
MODE ?= release
# List of essential files of the library.
LIB_MANDATORY = rdesc grammar

SRC_DIR = src
INC_DIR = include
DIST_DIR = dist
TARGET_DIR = $(DIST_DIR)/$(MODE)

OBJ_DIR = $(TARGET_DIR)/obj

# No need to change rules below this line.

include config.mk

# Compilation flags, selected based on MODE environment variable.
CFLAGS_release = $(CFLAGS_COMMON) -O2
CFLAGS_debug = $(CFLAGS_COMMON) -O0 -g3 -fsanitize=address
CFLAGS_test = $(CFLAGS_COMMON) -O0 -g3 --coverage

CFLAGS = $(CFLAGS_$(MODE))

ifeq ($(CFLAGS),)
$(error "WARNING: unknown mode $(MODE).")
endif

ifeq ($(FEATURES),full)
LIB = $(LIB_MANDATORY) $(FULL_FEATURES)
else
LIB = $(LIB_MANDATORY) $(FEATURES)
endif

SRCS = $(wildcard $(SRC_DIR)/*.c)

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
LIB_OBJS = $(foreach o, $(LIB), $(OBJ_DIR)/$o.o)

default: $(TARGET_DIR)/librdesc.so $(TARGET_DIR)/librdesc.a

PREFIX ?= /usr/local
install: $(TARGET_DIR)/librdesc.so $(TARGET_DIR)/librdesc.a
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/rdesc/

	install -m 755 $(TARGET_DIR)/librdesc.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(TARGET_DIR)/librdesc.a $(DESTDIR)$(PREFIX)/lib/

	install -m 644 include/* $(DESTDIR)$(PREFIX)/include/rdesc/


$(TARGET_DIR)/librdesc.a: $(LIB_OBJS) | $(TARGET_DIR)
	ar rcs $@ $^

$(TARGET_DIR)/librdesc.so: $(LIB_OBJS) | $(TARGET_DIR)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_DIR) $(OBJ_DIR):
	mkdir -p $@


clean:
	$(RM) $(DIST_DIR) docs

docs:
	doxygen


-include $(OBJS:.o=.d)

.PHONY: default clean docs install
