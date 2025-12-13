#include "../include/crous_arena.h"
#include <string.h>

typedef struct arena_chunk {
    struct arena_chunk *next;
    uint8_t *data;
    size_t size;
    size_t used;
} arena_chunk_t;

typedef struct {
    arena_chunk_t *head;
    size_t chunk_size;
    size_t total_used;
} arena_impl_t;

crous_arena* crous_arena_create(size_t chunk_size) {
    if (chunk_size == 0) chunk_size = 65536;
    
    arena_impl_t *impl = malloc(sizeof(*impl));
    if (!impl) return NULL;
    
    impl->head = malloc(sizeof(arena_chunk_t));
    if (!impl->head) {
        free(impl);
        return NULL;
    }
    
    impl->head->data = malloc(chunk_size);
    if (!impl->head->data) {
        free(impl->head);
        free(impl);
        return NULL;
    }
    
    impl->head->next = NULL;
    impl->head->size = chunk_size;
    impl->head->used = 0;
    impl->chunk_size = chunk_size;
    impl->total_used = 0;
    
    crous_arena *arena = malloc(sizeof(*arena));
    if (!arena) {
        free(impl->head->data);
        free(impl->head);
        free(impl);
        return NULL;
    }
    arena->_impl = impl;
    return arena;
}

void* crous_arena_alloc(crous_arena *arena, size_t size) {
    if (!arena || size == 0) return NULL;
    
    arena_impl_t *impl = (arena_impl_t *)arena->_impl;
    
    /* Try to allocate from current chunk */
    arena_chunk_t *chunk = impl->head;
    if (chunk->used + size <= chunk->size) {
        void *ptr = chunk->data + chunk->used;
        chunk->used += size;
        impl->total_used += size;
        return ptr;
    }
    
    /* Need new chunk */
    size_t new_chunk_size = impl->chunk_size;
    if (size > new_chunk_size) {
        new_chunk_size = size + 4096;
    }
    
    arena_chunk_t *new_chunk = malloc(sizeof(arena_chunk_t));
    if (!new_chunk) return NULL;
    
    new_chunk->data = malloc(new_chunk_size);
    if (!new_chunk->data) {
        free(new_chunk);
        return NULL;
    }
    
    new_chunk->next = impl->head;
    new_chunk->size = new_chunk_size;
    new_chunk->used = size;
    impl->head = new_chunk;
    impl->total_used += size;
    
    return new_chunk->data;
}

void crous_arena_reset(crous_arena *arena) {
    if (!arena) return;
    
    arena_impl_t *impl = (arena_impl_t *)arena->_impl;
    arena_chunk_t *chunk = impl->head;
    while (chunk) {
        chunk->used = 0;
        chunk = chunk->next;
    }
    impl->total_used = 0;
}

void crous_arena_free(crous_arena *arena) {
    if (!arena) return;
    
    arena_impl_t *impl = (arena_impl_t *)arena->_impl;
    arena_chunk_t *chunk = impl->head;
    while (chunk) {
        arena_chunk_t *next = chunk->next;
        free(chunk->data);
        free(chunk);
        chunk = next;
    }
    free(impl);
    free(arena);
}

size_t crous_arena_used(const crous_arena *arena) {
    if (!arena) return 0;
    arena_impl_t *impl = (arena_impl_t *)arena->_impl;
    return impl->total_used;
}
