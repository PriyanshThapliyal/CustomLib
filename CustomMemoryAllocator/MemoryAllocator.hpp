#pragma once

#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <pthread.h>
#include <cstring>
typedef char ALIGN[16];


// Header Block with count of size allocated
// with a is_free tag and pointer to next header block
// It is set to 16bytes to align 

union header {
    struct {
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub;
};

typedef union header header_t;


// Global thread lock
pthread_mutex_t global_malloc_lock = PTHREAD_MUTEX_INITIALIZER;

// Head and tail pointer for header block
header_t *head = nullptr, *tail = nullptr;

// Method used to check if there is free memory block that can be used
header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while(curr)
    {
        if(curr->s.is_free && curr->s.size >= size)
        {
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}

// Custom Memory Allocation
inline void* my_malloc(size_t size)
{   
    size_t total_size;
    void* block; // pointer to Memory block allocated
    header_t* block_header; // Pointer to Metadata 

    if(!size)
        return NULL;
        
    pthread_mutex_lock(&global_malloc_lock); // Locking thread

    block_header = get_free_block(size);  // Calling get_free_block() to get the memory address

    if(block_header)
    {
        block_header->s.is_free = 0;  // Set this block to not free
        pthread_mutex_unlock(&global_malloc_lock);
        return static_cast<void*>(block_header + 1); //Points to user's memory (just after metadata aka header)
    }

    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);   // Moves brk to the total size

    if(block == reinterpret_cast<void*>(-1))
    {
        pthread_mutex_unlock(&global_malloc_lock);
        return nullptr;
    }
    
    block_header = static_cast<header_t*>(block);   // Assign block pointer to block header 
    block_header->s.size = size;
    block_header->s.is_free = 0;
    block_header->s.next = NULL;
    
    if(!head)
        head = block_header;
    if(tail)
        tail->s.next = block_header;

    tail = block_header;
    pthread_mutex_unlock(&global_malloc_lock);

    return (void*)(block_header + 1);   // Points to the user memory
}

inline void my_free(void *block)
{
    header_t *block_header , *tmp;

    void *programbreak;

    if(!block)
        return;
    pthread_mutex_lock(&global_malloc_lock);
    block_header = static_cast<header_t*>(block) - 1;

    programbreak = sbrk(0); // Retrives the size
    if(static_cast<char*>(block) + block_header->s.size == programbreak)
    {
        if(head == tail)
        {
            head = tail = nullptr;
        }
        else 
        {
            tmp = head;
            while(tmp)
                {
                    if(tmp->s.next == tail)
                    {
                        tmp->s.next = nullptr;
                        tail = tmp;
                        break;
                    }
                    tmp = tmp->s.next;
                }
        }

        intptr_t shrink_size = sizeof(header_t) + block_header->s.size;     // Calculating size to shrink
        sbrk(-shrink_size);

        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }

    block_header->s.is_free = 1;    
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

    if(header->s.size >= size)
        return ptr;

    ret = my_malloc(size);

    if(ret)
    {
        std::memcpy(ret, ptr, header->s.size);     // Copying old memory block to new one
        my_free(ptr);     // Freeing old memory block
    }
    return ret;
}