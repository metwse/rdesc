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
 * @note Asserts on allocation failure.
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
 */
void rdesc_stack_reset(struct rdesc_stack **stack);

/**
 * @brief Returns pointer to the element at the specified index.
 *
 * @return Pointer to element at index i
 *
 * @pre i < rdesc_stack_len(stack)
 * @note Asserts on out-of-bounds access.
 * @note Index 0 is the bottom of the stack.
 */
void *rdesc_stack_at(struct rdesc_stack *stack, size_t index);

/**
 * @brief Applies a function to each element in the stack from bottom to top.
 *
 * @note Elements are visited in order from index 0 to len-1.
 * @note Do not modify the stack within the callback function.
 */
void rdesc_stack_foreach(struct rdesc_stack *stack, void fn(void *));

/**
 * @brief Ensures capacity for at least reserved_space additional elements.
 *
 * Reserved space is minimum number of additional elements to accommodate. The
 * implementation shall grow capacity until requirement is met.
 *
 * @note Does nothing if current capacity is already sufficient.
 */
void rdesc_stack_reserve(struct rdesc_stack **stack, size_t reserved_space);

/**
 * @brief Pushes multiple elements onto the stack.
 *
 * Elements can be source array of elements to copy, or NULL.
 *
 * Implementation should handle elements to be NULL. Stack is extended by count
 * elements without initialization in that case, and the returned pointer can
 * be used to initialize the allocated space manually.
 *
 * @return Pointer to the first pushed element in the stack
 *
 * @note If count is 0, this is a no-op and returns a pointer at current top.
 */
void *rdesc_stack_multipush(struct rdesc_stack **stack, void *elements, size_t count);

/**
 * @brief Pushes a single element onto the stack.
 *
 * This function has the exact behaivor with
 * `rdesc_stack_multipush(stack, element, 1)`.
 *
 * @see rdesc_stack_multipush
 */
void *rdesc_stack_push(struct rdesc_stack **stack, void *element);

/**
 * @brief Pops multiple elements from the stack, and returns a pointer to the
 *        first popped element ((new top + 1) after pop)
 *
 * @note If count is 0, this is a no-op and returns current top.
 */
void *rdesc_stack_multipop(struct rdesc_stack **stack, size_t count);

/**
 * @brief Removes and returns the top element from the stack.
 *
 * This function has the exact behaivor with `rdesc_stack_multipop(stack, 1)`.
 *
 * @see rdesc_stack_multipop
 */
void *rdesc_stack_pop(struct rdesc_stack **stack);

/** @brief Returns the top element without removing it. */
void *rdesc_stack_top(struct rdesc_stack *stack);

/** @brief Returns the current number of elements in the stack. */
size_t rdesc_stack_len(const struct rdesc_stack *stack);


#endif
