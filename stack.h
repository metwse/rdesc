/**
 * @file stack.h
 * @brief Token stack interface contract.
 *
 * This header defines the function signatures required to manipulate the
 * parser's backtracking stack.
 *
 * @note For custom implementations:
 *       If you have defined a custom `struct rdesc_stack` (by defining
 *       the `RDESC_STACK` macro externally), you **must** provide
 *       implementations for all the functions declared in this file. The
 *       parser engine (`librdesc`) relies on these exact signatures to perform
 *       backtracking.
 */

#ifndef RDESC_STACK_H
#define RDESC_STACK_H

#include "bnf.h"

#include <stddef.h>


struct rdesc_stack;


/** @brief Initializes the token stack. */
void rdesc_stack_init(struct rdesc_stack *s);

/** @brief Frees memory allocated by the token stack. */
void rdesc_stack_destroy(struct rdesc_stack *s);

/** @brief Pushes a token onto the stack. */
void rdesc_stack_push(struct rdesc_stack *s, struct bnf_token tk);

/** @brief Removes and returns the top token from the stack. */
struct bnf_token rdesc_stack_pop(struct rdesc_stack *s);

/** @brief Returns the underlying dynamic buffer. */
void *rdesc_stack_into_inner(struct rdesc_stack *s);

/** @brief Returns the underlying dynamic buffer. */
size_t rdesc_stack_len(const struct rdesc_stack *s);


#endif
