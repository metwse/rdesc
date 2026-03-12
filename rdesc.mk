# --- Build Configuration ---
#  Variables consisting entirely of uppercase letters are 'stable'
# configuration variables and can be modified or used outside of this Makefile
# (e.g. set via environment variables).

# Select features from 'stack', 'dump_cst', 'dump_bnf' or use 'full'.
RDESC_FEATURES ?= stack
# release, debug, or test
RDESC_MODE ?= release

# Directory containing rdesc source files.
RDESC_DIR ?= .

# rdesc public headers.
RDESC_INCLUDE_DIR := $(RDESC_DIR)/include

# private
rdesc_DIST_DIR := $(RDESC_DIR)/dist
rdesc_TARGET_DIR := $(rdesc_DIST_DIR)/$(RDESC_MODE)

# Target library objects.
RDESC_SO ?= $(rdesc_TARGET_DIR)/librdesc.so
RDESC ?= $(rdesc_TARGET_DIR)/librdesc.a


# Variables below this line are private.
# -----------------------------------------------------------------------------
rdesc_CFLAGS_COMMON := -std=c99 -Wall -Wextra -pedantic -fPIC

rdesc_CFLAGS_release := $(rdesc_CFLAGS_COMMON) -O2
rdesc_CFLAGS_debug := $(rdesc_CFLAGS_COMMON) -O0 -g3
rdesc_CFLAGS_test := $(rdesc_CFLAGS_COMMON) -O0 -g3 --coverage -DTEST_INSTRUMENTS

rdesc_CFLAGS := $(rdesc_CFLAGS_$(RDESC_MODE))

ifeq ($(rdesc_CFLAGS),)
$(error "WARNING: unknown mode $(RDESC_MODE).")
endif

# List of essential object files
rdesc_LIB_MANDATORY := rdesc grammar
# Object files linked if MODe is set to 'test'
rdesc_LIB_TEST := test_instruments

rdesc_ALL_FEATURES := stack dump_cst dump_bnf

rdesc_SRC_DIR := $(RDESC_DIR)/src

rdesc_OBJ_DIR := $(rdesc_TARGET_DIR)/obj

rdesc_MKDIR := $(or $(MKDIR),mkdir -p)

rdesc_OBJ_NAMES := $(rdesc_LIB_MANDATORY) \
	$(if $(filter $(RDESC_FEATURES),full),$(rdesc_ALL_FEATURES),$(RDESC_FEATURES)) \
	$(if $(filter $(RDESC_MODE),test),$(rdesc_LIB_TEST))

rdesc_OBJS := $(foreach o,$(rdesc_OBJ_NAMES),$(rdesc_OBJ_DIR)/$o.o)

$(RDESC) $(RDESC_SO): CFLAGS := $(rdesc_CFLAGS)

$(RDESC): $(rdesc_OBJS)
	$(AR) rcs $@ $^

$(RDESC_SO): $(rdesc_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(rdesc_OBJ_DIR)/%.o: $(rdesc_SRC_DIR)/%.c | $(rdesc_OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(rdesc_OBJ_DIR):
	$(rdesc_MKDIR) $@


-include $(rdesc_OBJS:.o=.d)
