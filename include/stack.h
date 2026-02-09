/**
 * @file stack.h
 * @brief Stack interface contract.
 *
 * This header defines the function signatures required to manipulate the
 * parser's backtracking stack.
 *
 * @note For custom implementations:
 *       If you have defined a custom `struct rdesc_stack`, you **must**
 *       provide implementations for all the functions declared in this file.
 *       The parser engine relies on these exact signatures to perform
 *       backtracking.
 */

#ifndef RDESC_STACK_H
#define RDESC_STACK_H

#include <stddef.h>

struct rdesc_stack;


/** @brief Initializes the stack. */
void rdesc_stack_init(struct rdesc_stack **stack, size_t element_size);

/** @brief Frees memory allocated by the stack. */
void rdesc_stack_destroy(struct rdesc_stack *stack);

/** @brief Cleans and reinitializes the stack. */
void rdesc_stack_reset(struct rdesc_stack **stack);

/** @brief Applies given function for every element of the stack. */
void rdesc_stack_foreach(struct rdesc_stack *stack, void fn(void *));

/**
 * @brief Increases the stack space, so that reserved space number of elements
 *        can fit without reallocations.
 */
void rdesc_stack_reserve(struct rdesc_stack **stack, size_t reserved_space);

/** @brief Pushes an element onto the stack. */
void *rdesc_stack_push(struct rdesc_stack **stack, void *element);

/**
 * @brief Pops count of elements and returns first element popped.
 *
 * @see rdesc_stack_pop
 */
void *rdesc_stack_multipop(struct rdesc_stack **stack, size_t count);

/**
 * @brief Removes and returns the top element from the stack.
 *
 * @return Return value is a pointer managed by the `stack`. Stack
 *         implementation shall guarantee life time of returned element: the
 *         pointer must live until any other push/pop/reset operation.
 *
 */
void *rdesc_stack_pop(struct rdesc_stack **stack);

/** @brief Returns the top element of the stack. */
void *rdesc_stack_top(struct rdesc_stack *s);

/** @brief Returns total number of elements of the stack. */
size_t rdesc_stack_len(const struct rdesc_stack *stack);


#endif
