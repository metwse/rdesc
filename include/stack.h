/**
 * @file stack.h
 * @brief Token stack interface contract
 *
 * This header defines the function signatures required to manipulate the
 * parser's backtracking stack.
 *
 * @note For custom implementations:
 *       If you have defined a custom `struct rdesc_stack`, you **must**
 *       provide implementations for all the functions declared in this file.
 *       The parser engine (`librdesc`) relies on these exact signatures to
 *       perform backtracking.
 */

#ifndef RDESC_STACK_H
#define RDESC_STACK_H

#include "rdesc.h"

#include <stddef.h>


struct rdesc_stack;


/** @brief Initializes the token stack. */
void rdesc_stack_init(struct rdesc_stack **stack, size_t token_size);

/** @brief Frees memory allocated by the token stack. */
void rdesc_stack_destroy(struct rdesc_stack *stack);

/** @brief Clean ups elements in the stack.
 *
 * Calls token destroyer method for each token IF set.
 */
void rdesc_stack_reset(struct rdesc_stack **stack,
		       rdesc_tk_destroyer_func token_destroyer,
		       size_t token_size);

/** @brief Pushes a token onto the stack. */
void rdesc_stack_push(struct rdesc_stack **stack,
		      struct rdesc_token *token,
		      size_t token_size);

/** @brief Removes and returns the top token from the stack. */
struct rdesc_token *rdesc_stack_pop(struct rdesc_stack **stack,
				   size_t token_size);

/** @brief Returns total number of elements of the stack. */
size_t rdesc_stack_len(const struct rdesc_stack *stack);


#endif
