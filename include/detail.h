/* Public-facing implementation details (unstable). */

#ifndef RDESC_DETAIL_H
#define RDESC_DETAIL_H
/** @cond */

#include <stddef.h>
#include <stdint.h>


// To suppress clangd's false-positive pragma warning
// See https://github.com/clangd/clangd/issues/1167
void _rdesc_priv_dummy_declaration(void);


#if defined(__GNUC__) || defined(__clang__)
#define _rdesc_wur __attribute__((warn_unused_result))
#else
#define _rdesc_wur
#endif


/* These structs are private and should only be accessed via the provided
 * CST macros. */

#pragma pack(push, 1)

struct _rdesc_priv_tk {
	uint16_t _pad : 1;
	uint16_t id : 15  /* Token identifier (0 reserved, 1-32767 valid). */;

	uint32_t seminfo  /* Semantic info starts here and extends into
			   * the flexible array member in _rdesc_priv_node. */;
};

struct _rdesc_priv_nt {
	uint16_t _pad : 1;
	uint16_t id : 15  /* 0 is NOT reserved unlike token ids. */;

	uint16_t variant;
	uint16_t child_count;
};

struct _rdesc_priv_node {
	/* ALSO CHANGE sizeof_node macro for any change in this struct. */
	size_t parent  /* Index of parent. */;
	uint16_t unwind_size  /* Previous node's unwind size (for backward
		               * navigation on the stack). */;

	union {
		uint16_t ty : 1  /* 0 for token and 1 for nonterminal. */;

		struct _rdesc_priv_tk tk;
		struct _rdesc_priv_nt nt;
	} n;
};

#pragma pack(pop)


#endif
/** @cond */
