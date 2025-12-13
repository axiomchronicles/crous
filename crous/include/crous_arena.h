#ifndef CROUS_ARENA_H
#define CROUS_ARENA_H

#include "crous_types.h"
#include <stdlib.h>

/* ============================================================================
   MEMORY ARENA
   ============================================================================ */

/**
 * Arena allocator for efficient memory management
 * Allocates memory in chunks and frees all at once
 */

/* Opaque arena structure - implementation in arena.c */
typedef struct {
    void *_impl;
} crous_arena;

/**
 * Create a new arena with initial chunk size
 */
crous_arena* crous_arena_create(size_t chunk_size);

/**
 * Allocate memory from arena
 */
void* crous_arena_alloc(crous_arena *arena, size_t size);

/**
 * Reset arena (keep structure but free all allocations)
 */
void crous_arena_reset(crous_arena *arena);

/**
 * Free arena and all allocations
 */
void crous_arena_free(crous_arena *arena);

/**
 * Get total memory used by arena
 */
size_t crous_arena_used(const crous_arena *arena);

#endif /* CROUS_ARENA_H */
