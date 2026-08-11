#include <iostream>
#include "MemoryAllocator.hpp"

int main() {
    std::cout << "--- Starting Complete Allocator Test Suite ---\n\n";

    // ---------------------------------------------------------
    // TEST 1: my_malloc and my_free
    // ---------------------------------------------------------
    std::cout << "[Test 1] Testing my_malloc...\n";
    int* val = static_cast<int*>(my_malloc(sizeof(int)));
    *val = 99;
    
    if (*val == 99) {
        std::cout << "  -> PASSED: Memory allocated and value stored successfully.\n";
    } else {
        std::cout << "  -> FAILED: Value mismatch.\n";
    }
    my_free(val);
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 2: my_calloc (Zero Initialization)
    // ---------------------------------------------------------
    std::cout << "[Test 2] Testing my_calloc (Zero Initialization)...\n";
    size_t array_size = 5;
    int* zero_array = static_cast<int*>(my_calloc(array_size, sizeof(int)));
    
    bool all_zeroes = true;
    for (size_t i = 0; i < array_size; i++) {
        if (zero_array[i] != 0) {
            all_zeroes = false;
            break;
        }
    }
    
    if (all_zeroes) {
        std::cout << "  -> PASSED: Array successfully allocated and zeroed out.\n";
    } else {
        std::cout << "  -> FAILED: Memory contains garbage values.\n";
    }
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 3: my_realloc (Growing a memory block)
    // ---------------------------------------------------------
    std::cout << "[Test 3] Testing my_realloc (Growing block)...\n";
    
    // Assign 1, 2, 3, 4, 5 to our previously calloc'd array
    for (size_t i = 0; i < array_size; i++) {
        zero_array[i] = i + 1; 
    }
    
    // Reallocate array to hold 10 integers instead of 5
    int* grown_array = static_cast<int*>(my_realloc(zero_array, 10 * sizeof(int)));
    
    // Verify old data survived the transfer
    bool data_survived = true;
    for (size_t i = 0; i < array_size; i++) {
        if (grown_array[i] != (int)(i + 1)) {
            data_survived = false;
            break;
        }
    }
    
    if (data_survived) {
        std::cout << "  -> PASSED: Block grown and old data preserved safely.\n";
    } else {
        std::cout << "  -> FAILED: Old data was corrupted during realloc.\n";
    }
    std::cout << "\n";

    // ---------------------------------------------------------
    // TEST 4: my_realloc Edge Cases
    // ---------------------------------------------------------
    std::cout << "[Test 4] Testing my_realloc (Edge Cases)...\n";
    
    // Edge Case A: realloc(nullptr, size) should act like malloc
    void* initial_realloc = my_realloc(nullptr, sizeof(int));
    if (initial_realloc != nullptr) {
        std::cout << "  -> PASSED: realloc(nullptr, size) correctly acted as malloc.\n";
    } else {
        std::cout << "  -> FAILED: realloc(nullptr, size) returned nullptr.\n";
    }
    
    // Edge Case B: realloc(ptr, 0) should act like free
    void* freed_by_realloc = my_realloc(initial_realloc, 0);
    if (freed_by_realloc == nullptr) {
        std::cout << "  -> PASSED: realloc(ptr, 0) correctly acted as free.\n";
    } else {
        std::cout << "  -> FAILED: realloc(ptr, 0) did not return nullptr.\n";
    }

    // Clean up
    my_free(grown_array);

    std::cout << "\n--- All Tests Completed ---\n";
    return 0;
}