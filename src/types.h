/**
 * @file types.h
 * @brief
 */

#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>


#define tk_size(p) (sizeof(struct rdesc_tk) - sizeof(uint32_t) + (p).seminfo_size)
#define nt_size(child_cap) (sizeof(struct rdesc_nt) + sizeof(size_t) * child_cap)

#define node_size(p) (tk_size(p) > nt_size(0) ? tk_size(p) : nt_size(0))


struct rdesc_tk {
	uint32_t _pad : 1;
	uint32_t id : 31;

	uint32_t seminfo;
};

struct rdesc_nt {
	uint32_t _pad : 1;
	uint32_t id : 31;

	uint16_t variant;
	uint16_t child_count;
};

struct rdesc_node {
	size_t parent;

	union {
		uint32_t ty : 1;

		struct rdesc_tk tk;
		struct rdesc_nt nt;
	} n;

	uint8_t _[];
};


#endif
