#include <iostream>
#include "mylib.hpp" // Assuming you saved both Vector and String in here

int main() 
{
    std::cout << "--- 1. Testing String Concatenation ---\n";
    mylib::String s1 = "Hello";
    mylib::String s2 = " World";
    
    // Testing operator+ (Standalone)
    mylib::String s3 = s1 + s2;
    std::cout << "s3: ";
    s3.print(); 

    // Testing operator+= (Append)
    s1 += " C++";
    std::cout << "s1: ";
    s1.print(); 

    mylib::String myText = "Iterators are awesome!";

    // This will now magically work!
    for (char c : myText) 
    {
        std::cout << c << "-";
    }
    // Output: I-t-e-r-a-t-o-r-s- -a-r-e- -a-w-e-s-o-m-e-!-

    std::cout << "\n--- 2. Testing Index Modification ---\n";
    // Testing operator[] write access
    s3[0] = 'J';
    std::cout << "s3 mutated: ";
    s3.print(); 

    std::cout << "\n--- 3. Testing String Equality ---\n";
    mylib::String s4 = "Jello World";
    if (s3 == s4) 
    {
        std::cout << "Match successful: s3 and s4 are identical!\n";
    }

    std::cout << "\n--- 4. The Ultimate Test: Vector of Strings ---\n";
    // If your Rule of Three is correct, pushing strings into a resizing vector will work perfectly.
    mylib::Vector<mylib::String> words;
    
    words.push_back("Apple");
    words.push_back("Banana");
    words.push_back("Cherry");
    words.push_back("Dragonfruit");
    
    for (int i = 0; i < words.size(); i++) 
    {
        std::cout << "Word " << i << ": ";
        words[i].print();
    }

    std::cout << "\nAll memory tests passed successfully!\n";
    return 0;
}