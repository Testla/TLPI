// This will be a non-thread-safe version.
// Try to operate at page boundary.
// XXX: We rely on the assumption that
// program break is always same as the end of the last block.
// So actually we assume exclusive control of program break.
#include "7-2.h"

#include <unistd.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct block {
    // Including the size of length itself.
    size_t length;
    struct block *prev;
    struct block *next;
} block;

// To keep the implementation simple,
// we preallocate a head node for the free list.
// Keep the length of the head node 0.
static block free_list_head = {0, NULL, NULL}; 

static long Page_size = -1;

static const size_t
    Minimum_allocate_page = 32,
    Minimum_surplus_page = 1,
    Minimum_release_page = 32;

void *malloc_(size_t size) {
    if (Page_size == -1) {
        Page_size = sysconf(_SC_PAGESIZE);
        if (Page_size == -1) {
            return NULL;
        }
    }

    size = MAX(size, sizeof(block) - sizeof(size_t));

    block *use = &free_list_head;
    while (use->length < sizeof(use->length) + size && use->next) {
        use = use->next;
    }

    if (use->length < sizeof(use->length) + size) {
        // No suitable free block, allocate more space.
        void *previous_break = sbrk(0);
        // XXX: Assume no arithmetic overflow will happen.
        size_t allocate_size = MAX(
                size + Minimum_surplus_page * Page_size,
                Minimum_allocate_page * Page_size);
        if (previous_break == use + use->length) {
            // We can use the last free block.
            allocate_size -= use->length;
        }
        // Align new program break to the next page boundary.
        size_t actual_allocate_size =
            ((intptr_t)previous_break + allocate_size + Page_size - 1)
            / Page_size * Page_size
            - (intptr_t)previous_break;
        if (sbrk(actual_allocate_size) == (void *)-1) {
            return NULL;
        }

        if (previous_break == use + use->length) {
            // The current block is next to end of the heap.
            use->length += actual_allocate_size;
        } else {
            block *new_block = (block *)previous_break;
            use->next = new_block;
            new_block->length = actual_allocate_size;
            new_block->next = NULL;
            new_block->prev = use;
            use = new_block;
        }
    }

    if (use->length < sizeof(use->length) + size + sizeof(block)) {
        // Do not split the block because surplus space can't hold a block.
        use->prev->next = use->next;
        if (use->next) {
            use->next->prev = use->prev;
        }
    } else {
        // Split the block.
        block *new_block = (block *)((char *)use + sizeof(use->length) + size);
        use->prev->next = new_block;
        new_block->length = use->length - sizeof(use->length) - size;
        use->length = sizeof(use->length) + size;
        new_block->next = use->next;
        new_block->prev = use->prev;
        if (use->next) {
            use->next->prev = new_block;
        }
    }
    return (char *)use + sizeof(use->length);
}

void free_(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    // Insert the block into free list.
    block *new_block = (block *)((char *)ptr - sizeof(size_t)), *prev = &free_list_head, *next;
    while (prev->next && new_block > prev->next) {
        prev = prev->next;
    }
    next = prev->next;
    new_block->next = next;
    if (next) {
        next->prev = new_block;
    }
    prev->next = new_block;
    new_block->prev = prev;

    if ((char *)prev + prev->length == (char *)new_block) {
        // Merge into prev.
        prev->length += new_block->length;
        prev->next = next;
        if (next) {
            next->prev = prev;
        }
        new_block = prev;
    }

    if ((char *)new_block + new_block->length == (char *)next) {
        // Merge next.
        new_block->length += next->length;
        new_block->next = next->next;
        if (new_block->next) {
            new_block->next->prev = new_block;
        }
    }

    // Adjust program break when neccessary.

    void *program_break = sbrk(0);

    if ((char *)new_block + new_block->length == program_break) {
        size_t num_release_page = new_block->length / Page_size;
        size_t remaining_length = new_block->length - num_release_page;
        if (0 < remaining_length && remaining_length < sizeof(block)) {
            num_release_page -= 1;
            remaining_length += Page_size;
        }
        if (num_release_page >= Minimum_release_page) {
            new_block->length -= num_release_page * Page_size;
            if (new_block->length == 0) {
                new_block->prev->next = new_block->next;
            }
            sbrk(-num_release_page * Page_size);
        }
    }
}
