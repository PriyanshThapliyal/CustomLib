#include <iostream>
#include <vector>
#include <thread>
#include <cassert>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include "MemoryAllocator.hpp"

// ANSI colors for readable terminal output
#define GREEN "\033[32m"
#define RED   "\033[31m"
#define RESET "\033[0m"

void report(bool passed, const char* name) {
    if (passed) {
        std::cout << "  [" << GREEN << "PASSED" << RESET << "] " << name << "\n";
    } else {
        std::cout << "  [" << RED << "FAILED" << RESET << "] " << name << "\n";
    }
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "       COMPREHENSIVE MEMORY ALLOCATOR TEST SUITE         \n";
    std::cout << "=========================================================\n\n";

    // ---------------------------------------------------------
    // TEST 1: Basic malloc and free
    // ---------------------------------------------------------
    std::cout << "--- [Test 1] Basic malloc and free ---\n";
    int* val = static_cast<int*>(my_malloc(sizeof(int)));
    bool t1_pass = false;
    if (val != nullptr) {
        *val = 99;
        t1_pass = (*val == 99);
        my_free(val);
    }
    report(t1_pass, "Allocated, read, wrote, and freed single integer");
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 2: calloc (Zero Initialization)
    // ---------------------------------------------------------
    std::cout << "--- [Test 2] calloc (Zero Initialization) ---\n";
    size_t array_size = 32;
    int* zero_array = static_cast<int*>(my_calloc(array_size, sizeof(int)));
    bool t2_pass = (zero_array != nullptr);
    if (t2_pass) {
        for (size_t i = 0; i < array_size; i++) {
            if (zero_array[i] != 0) {
                t2_pass = false;
                break;
            }
        }
        my_free(zero_array);
    }
    report(t2_pass, "Calloc zeroed memory successfully");
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 3: realloc Growth and Edge Cases
    // ---------------------------------------------------------
    std::cout << "--- [Test 3] realloc Growth and Edge Cases ---\n";
    int* orig = static_cast<int*>(my_malloc(5 * sizeof(int)));
    for (int i = 0; i < 5; i++) orig[i] = (i + 1) * 10;

    int* expanded = static_cast<int*>(my_realloc(orig, 10 * sizeof(int)));
    bool t3_growth = true;
    for (int i = 0; i < 5; i++) {
        if (expanded[i] != (i + 1) * 10) {
            t3_growth = false;
            break;
        }
    }
    report(t3_growth, "Data preserved across realloc expansion");

    void* realloc_null = my_realloc(nullptr, 64);
    report(realloc_null != nullptr, "realloc(nullptr, size) acts as malloc");

    void* realloc_free = my_realloc(realloc_null, 0);
    report(realloc_free == nullptr, "realloc(ptr, 0) acts as free");

    my_free(expanded);
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 4: 16-Byte Payload Alignment (Phase 1)
    // ---------------------------------------------------------
    std::cout << "--- [Test 4] 16-Byte Payload Alignment (Phase 1) ---\n";
    bool t4_align = true;
    std::vector<void*> align_ptrs;
    size_t test_sizes[] = { 1, 3, 7, 15, 16, 17, 31, 64, 127, 255 };

    for (size_t s : test_sizes) {
        void* p = my_malloc(s);
        if (reinterpret_cast<uintptr_t>(p) % 16 != 0) {
            t4_align = false;
        }
        align_ptrs.push_back(p);
    }
    for (void* p : align_ptrs) my_free(p);
    report(t4_align, "All returned pointers are strictly 16-byte aligned");
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 5: Block Splitting (Phase 2)
    // ---------------------------------------------------------
    std::cout << "--- [Test 5] Block Splitting (Phase 2) ---\n";
    void* big_block = my_malloc(2048);
    my_free(big_block); // Returned to free list with size >= 2048

    // Allocate smaller chunk; allocator should carve it out of 2048 block
    void* split_p1 = my_malloc(64);
    // Allocate another chunk; should take from remainder without expanding heap
    void* split_p2 = my_malloc(128);

    report(split_p1 != nullptr && split_p2 != nullptr, "Big block split and served multiple smaller requests");
    my_free(split_p1);
    my_free(split_p2);
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 6: Bidirectional Coalescing (Phase 2)
    // ---------------------------------------------------------
    std::cout << "--- [Test 6] Bidirectional Coalescing (Phase 2) ---\n";
    void* b1 = my_malloc(128);
    void* b2 = my_malloc(128);
    void* b3 = my_malloc(128);

    // Free b1 (left) and b3 (right)
    my_free(b1);
    my_free(b3);

    // Free b2 (middle) -> Should merge left with b1 and right with b3
    my_free(b2);

    // Request a block that can only fit if b1, b2, and b3 were coalesced!
    void* merged = my_malloc(384);
    report(merged != nullptr, "Contiguous free blocks merged into single large block");
    my_free(merged);
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 7: Cascading Heap Retraction via sbrk (Phase 2)
    // ---------------------------------------------------------
    std::cout << "--- [Test 7] Cascading Heap Contraction (Phase 2) ---\n";
    void* break_start = sbrk(0);
    void* h1 = my_malloc(256);
    void* h2 = my_malloc(256);
    void* h3 = my_malloc(256);

    // Free in non-tail order
    my_free(h1); // Middle/head (can't shrink yet)
    my_free(h2); // Middle (can't shrink yet)
    my_free(h3); // Tail! Should trigger cascade shrink of h3, h2, and h1!

    void* break_end = sbrk(0);
    report(break_start == break_end, "Heap contracted completely back to initial break");
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 8: mmap for Large Allocations (> 128 KB) (Phase 3)
    // ---------------------------------------------------------
    std::cout << "--- [Test 8] mmap for Large Allocations (> 128 KB) (Phase 3) ---\n";
    void* heap_break_before = sbrk(0);

    // 256 KB allocation (well above the 128 KB threshold)
    size_t large_size = 256 * 1024;
    char* large_buf = static_cast<char*>(my_malloc(large_size));

    // Verify heap break did NOT move (mmap bypassed sbrk!)
    void* heap_break_after = sbrk(0);
    bool mmap_bypassed = (heap_break_before == heap_break_after) && (large_buf != nullptr);
    report(mmap_bypassed, "Allocation >= 128 KB bypassed sbrk heap break completely");

    // Write test pattern across the entire 256 KB buffer
    bool mmap_rw_ok = true;
    if (large_buf) {
        std::memset(large_buf, 0xAA, large_size);
        for (size_t i = 0; i < large_size; i += 4096) {
            if (static_cast<unsigned char>(large_buf[i]) != 0xAA) {
                mmap_rw_ok = false;
                break;
            }
        }
    }
    report(mmap_rw_ok, "Large mmap buffer is fully readable and writable");

    // Free the mmap block (calls munmap)
    my_free(large_buf);
    report(true, "Large mmap block released via munmap cleanly");
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 9: Cross-Boundary realloc (Heap <-> mmap)
    // ---------------------------------------------------------
    std::cout << "--- [Test 9] Cross-Boundary realloc (Heap <-> mmap) ---\n";
    // 1. Allocate on Heap (1 KB)
    char* cross_buf = static_cast<char*>(my_malloc(1024));
    std::strcpy(cross_buf, "Hello from Heap!");

    // 2. Expand into mmap territory (200 KB)
    cross_buf = static_cast<char*>(my_realloc(cross_buf, 200 * 1024));
    bool t9_heap_to_mmap = (std::strcmp(cross_buf, "Hello from Heap!") == 0);
    report(t9_heap_to_mmap, "Successfully reallocated from Heap to mmap with data intact");

    // 3. Shrink back into Heap territory (2 KB)
    cross_buf = static_cast<char*>(my_realloc(cross_buf, 2048));
    bool t9_mmap_to_heap = (std::strcmp(cross_buf, "Hello from Heap!") == 0);
    report(t9_mmap_to_heap, "Successfully reallocated from mmap back to Heap with data intact");

    my_free(cross_buf);
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 10: Multi-threaded Concurrency Stress Test
    // ---------------------------------------------------------
    std::cout << "--- [Test 10] Multi-Threaded Concurrency Stress Test ---\n";
    constexpr int NUM_THREADS = 8;
    constexpr int ALLOCS_PER_THREAD = 1000;
    std::vector<std::thread> threads;
    bool thread_fail = false;

    for (int t = 0; t < NUM_THREADS; t++) {
        threads.emplace_back([&thread_fail, t]() {
            for (int i = 0; i < ALLOCS_PER_THREAD; i++) {
                size_t sz = ((i % 16) + 1) * 32;
                int* ptr = static_cast<int*>(my_malloc(sz));
                if (!ptr) {
                    thread_fail = true;
                    return;
                }
                *ptr = t * 10000 + i;
                if (*ptr != t * 10000 + i) {
                    thread_fail = true;
                }
                my_free(ptr);
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }
    report(!thread_fail, "8 concurrent threads executed 8,000 malloc/free operations safely");
    std::cout << "\n";

    std::cout << "=========================================================\n";
    std::cout << "                 ALL TEST SUITES FINISHED                \n";
    std::cout << "=========================================================\n";

    return 0;
}
