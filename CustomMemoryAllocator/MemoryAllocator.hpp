#pragma once

#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <pthread.h>
#include <cstring>

constexpr size_t ALIGNMENT = 16;

// Align up round up input byte to multiple of 16
inline size_t align_up(size_t size)
{
  if(size > SIZE_MAX - (ALIGNMENT - 1)) return 0; // Signals overflow of size
  return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1); 
}

// Header Block with count of size allocated
// with a is_free tag and pointer to next header block
// It is set to 16bytes to align 

struct alignas(16) header{
        size_t size;
        unsigned is_free;
        header *next;
        header *prev;
};

typedef  header header_t;


// Global thread lock
inline pthread_mutex_t global_malloc_lock = PTHREAD_MUTEX_INITIALIZER;

// Head and tail pointer for header block
inline header_t *head = nullptr, *tail = nullptr;


// Method for splitting the block
inline void split_block(header_t *block_header, size_t size)
{
    if(block_header->size >= size + sizeof(header_t) + ALIGNMENT)
    {
        header_t *new_block = reinterpret_cast<header_t*>(reinterpret_cast<char*>(block_header + 1) + size);

        new_block->size = block_header->size - size - sizeof(header_t);
        new_block->is_free = 1;
        new_block->prev = block_header;
        new_block->next = block_header->next;

        if(new_block->next)
            new_block->next->prev = new_block;
        else
            tail = new_block;

        block_header->next = new_block;
        block_header->size = size;
    }
}

// Method to merge two memory block 
inline header_t* merge_block(header_t *block_header)
{
    // Merge in next if block is free
    if(block_header->next && block_header->next->is_free)
    {
        block_header->size += sizeof(header_t) + block_header->next->size;
        block_header->next = block_header->next->next;

        if(block_header->next)
            block_header->next->prev = block_header;
        else
            tail = block_header;
    }

    if(block_header->prev && block_header->prev->is_free)
    {
        block_header->prev->size += sizeof(header_t) + block_header->size;
        block_header->prev->next = block_header->next;
        if(block_header->next)
            block_header->next->prev = block_header->prev;
        else 
            tail = block_header->prev;
    }
    if(block_header->prev && block_header->prev->is_free)
        block_header = block_header->prev;
    return block_header;

}
// Method used to check if there is free memory block that can be used
inline header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while(curr)
    {
        if(curr->is_free && curr->size >= size)
        {
            split_block(curr,size); 
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// Custom Memory Allocation
inline void* my_malloc(size_t size)
{   
    size = align_up(size);
    void* block; // pointer to Memory block allocated
    header_t* block_header; // Pointer to Metadata 
                            
    if(!size)
        return NULL;
        
    pthread_mutex_lock(&global_malloc_lock); // Locking thread

    block_header = get_free_block(size);  // Calling get_free_block() to get the memory address

    if(block_header)
    {
        block_header->is_free = 0;  // Set this block to not free
        pthread_mutex_unlock(&global_malloc_lock);
        return static_cast<void*>(block_header + 1); //Points to user's memory (just after metadata aka header)
    }

    size_t total_size = sizeof(header_t) + size;

    if(!head)
    {
      uintptr_t current_break = (uintptr_t)sbrk(0);
      uintptr_t aligned_break = align_up(current_break);

      if(aligned_break - current_break > 0)
      {
        sbrk(aligned_break - current_break);
      }
    }
    block = sbrk(total_size);

    if(block == reinterpret_cast<void*>(-1))
    {
        pthread_mutex_unlock(&global_malloc_lock);
        return nullptr;
    }
    
    block_header = static_cast<header_t*>(block);   // Assign block pointer to block header 
    block_header->size = size;
    block_header->is_free = 0;
    block_header->next = NULL;
    block_header->prev = tail;

    if(!head)
        head = block_header;
    if(tail)
        tail->next = block_header;

    tail = block_header;
    pthread_mutex_unlock(&global_malloc_lock);

    return (void*)(block_header + 1);   // Points to the user memory
}

inline void my_free(void *block)
{
    if(!block)
        return;

    pthread_mutex_lock(&global_malloc_lock);
    header_t* block_header = static_cast<header_t*>(block) - 1;

    block_header->is_free = 1; // Marks as free

    // Cascade with adjacent free neighbour blocks
    block_header = merge_block(block_header);

    // Cascade shrink as many trailing free block as possible
    while(tail && tail->is_free && (static_cast<char*>(static_cast<void*>(tail + 1)) + tail->size == sbrk(0)))
    {
        intptr_t shrink_size = sizeof(header_t) + tail->size;
        tail = tail->prev;
        if(tail) tail->next = nullptr;
        else head = nullptr;
        sbrk(-shrink_size);
    }
    pthread_mutex_unlock(&global_malloc_lock);
}

inline void* my_calloc(size_t num, size_t nsize)
{
    size_t size;

    if(!num || !nsize)
        return nullptr;

    size = nsize * num;

    // Check for multiplicative overflow
    if(nsize != size / num)
        return nullptr;

    void* block = my_malloc(size);

    if(!block)  return nullptr;
    std::memset(block, 0, size);
    return block;
}

inline void *my_realloc(void *ptr, size_t size)
{
    size = align_up(size);
    header_t* header;
    void* ret;

    if(!ptr)
        return my_malloc(size);

    if(!size)
    {
        my_free(ptr);
        return nullptr;
    }
    header = static_cast<header_t*>(ptr) - 1;

    if(header->size >= size)
        return ptr;

    ret = my_malloc(size);

    if(ret)
    {
        std::memcpy(ret, ptr, header->size);     // Copying old memory block to new one
        my_free(ptr);     // Freeing old memory block
    }
    return ret;
}
