/**
 * @file stack.h
 * @brief Generic dynamic stack interface contract.
 *
 * This header defines the function signatures required to manipulate the
 * parser's stack. The stack is a generic, type-agnostic container that stores
 * fixed-size elements and provides dynamic growth/shrink capabilities.
 *
 * ## Custom Implementations
 * If you have defined a custom `struct rdesc_stack`, you **must** provide
 * implementations for all functions declared in this file. The parser engine
 * relies on these exact signatures.
 *
 * ## Memory Management
 * The stack owns all memory it allocates. Pointers returned by stack
 * operations (push, pop, top, at) remain valid until the next stack-modifying
 * operation (push, pop, reset, destroy).
 */

#ifndef RDESC_STACK_H
#define RDESC_STACK_H

#include <stddef.h>

struct rdesc_stack;


/**
 * @brief Initializes a new stack with the specified element size.
 *
 * @param stack Pointer to stack pointer (**will be allocated**)
 * @param element_size Size of each element in bytes
 *
 * @post Stack is allocated with initial capacity and zero length.
 * @note *stack set to NULL if allocation failed.
 */
void rdesc_stack_init(struct rdesc_stack **stack, size_t element_size);

/**
 * @brief Frees all memory allocated by the stack.
 *
 * @post All memory is released. Stack pointer becomes invalid.
 */
void rdesc_stack_destroy(struct rdesc_stack *stack);

/**
 * @brief Clears the stack and resets capacity to initial size.
 *
 * @post Stack length is zero, capacity is reset to initial value.
 * @note *stack set to NULL if allocation failed.
 */
void rdesc_stack_reset(struct rdesc_stack **stack);

/**
 * @brief Returns pointer to the element at the specified index.
 *
 * @return Pointer to element at index i
 *
 * @pre i < rdesc_stack_len(stack)
 *
 * @note Index 0 is the bottom of the stack.
 */
void *rdesc_stack_at(struct rdesc_stack *stack, size_t index);

/**
 * @brief Pushes multiple elements onto the stack.
 *
 * `elements` is array of elements to copy, or NULL to allocate uninitialized
 * space.
 *
 * @return Pointer to the first pushed element in the stack, i.e., pointer
 *         to allocated space for manual initialization.
 *
 * @note If count is 0, this is a no-op and returns a pointer at current top.
 * @note Return NULL if memory allocation is failed. Elements are not pushed
 *       in case of failure.
 * @warning As this function may trigger realloc, the `element` SHALL NOT point
 *          the stack.
 */
void *rdesc_stack_multipush(struct rdesc_stack **stack, void *elements, size_t count);

/**
 * @brief Pushes a single element onto the stack.
 *
 * This function has the exact behavior with
 * `rdesc_stack_multipush(stack, element, 1)`.
 *
 * @see rdesc_stack_multipush
 */
void *rdesc_stack_push(struct rdesc_stack **stack, void *element);

/**
 * @brief Pops multiple elements from the stack, and returns pointer to the
 *        element that was at the bottom of the popped range.
 *
 *
 * @note This may trigger realloc, return NULL if memory realloc is failed. The
 *       stack should pop elements even if it failed to realloc.
 * @note If count is 0, this is a no-op and returns current top.
 */
void *rdesc_stack_multipop(struct rdesc_stack **stack, size_t count);

/**
 * @brief Removes and returns the top element from the stack.
 *
 * This function has the exact behavior with `rdesc_stack_multipop(stack, 1)`.
 *
 * @see rdesc_stack_multipop
 */
void *rdesc_stack_pop(struct rdesc_stack **stack);

/** @brief Returns the top element without removing it. */
void *rdesc_stack_top(struct rdesc_stack *stack);

/** @brief Returns the current number of elements in the stack. */
size_t rdesc_stack_len(const struct rdesc_stack *stack);


#endif
