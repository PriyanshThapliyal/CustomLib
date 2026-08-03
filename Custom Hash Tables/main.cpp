#include "HashTables.hpp"

int main()
{
std::cout << "--- 1. Testing Basic Insert and Get ---\n";
    c_unordered_map<std::string, int> scores(4); // Start small to force a rehash soon
    
    scores.insert("Alice", 100);
    scores.insert("Bob", 85);
    scores.insert("Charlie", 92);
    
    std::cout << "Alice's score: " << scores.get("Alice").value_or(-1) << "\n";
    std::cout << "Bob's score: " << scores["Bob"] << "\n"; // Testing operator[]
    std::cout << "David's score (not found): " << scores.get("David").value_or(-1) << "\n";


    std::cout << "\n--- 2. Testing Rehash (Forcing Load Factor > 1.0) ---\n";
    std::cout << "Current size: " << scores.size() << "\n";
    scores.insert("David", 77);
    scores.insert("Eve", 95); // This 5th insert should trigger the rehash!
    std::cout << "New size after rehash triggers: " << scores.size() << "\n";


    std::cout << "\n--- 3. Testing Iterators ---\n";
    std::cout << "All current scores in the map:\n";
    for(const auto& kv : scores)
    {
        std::cout << "  Key: " << kv.first << ", Value: " << kv.second << "\n";
    }


    std::cout << "\n--- 4. Testing Erase ---\n";
    std::cout << "Erasing 'Bob'...\n";
    scores.erase("Bob");
    if(!scores.get("Bob").has_value()) {
        std::cout << "Bob was successfully erased. Current size: " << scores.size() << "\n";
    }


    std::cout << "\n--- 5. Testing Rule of 5 (Copy Semantics) ---\n";
    c_unordered_map<std::string, int> scores_copy = scores; // Triggers Copy Constructor
    
    // Modify the copy to prove it's a deep copy, not shared memory
    scores_copy["Alice"] = 999; 
    
    std::cout << "Original Alice: " << scores["Alice"] << " (Should still be 100)\n";
    std::cout << "Copied Alice: " << scores_copy["Alice"] << " (Should be 999)\n";
    std::cout << "Are maps equal? " << (scores == scores_copy ? "Yes" : "No") << "\n";


    std::cout << "\n--- 6. Testing Rule of 5 (Move Semantics) ---\n";
    c_unordered_map<std::string, int> stolen_scores = std::move(scores_copy); // Triggers Move Constructor
    
    std::cout << "Stolen Alice score: " << stolen_scores["Alice"] << "\n";
    std::cout << "Size of moved-from map: " << scores_copy.size() << " (Should be 0)\n";

    std::cout << "\nAll tests completed successfully!\n";

    return 0;
}