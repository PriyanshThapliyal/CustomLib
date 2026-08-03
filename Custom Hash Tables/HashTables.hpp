#pragma once

#include <iostream>
#include <cstring>
#include <list>
#include <optional>
#include <vector>
#include <string>

template <typename K, typename V>
struct HashNode
{
    K key;
    V value;
    HashNode* next = nullptr;
    HashNode(const K& k, const V& v, HashNode* nxt) : key(k), value(v), next(nxt) {}
};

template <typename K, typename V, typename Hash = std::hash<K>> 
class c_unordered_map
{
    public:
        c_unordered_map(size_t initial_capacity = 16)
        {
            capacity = initial_capacity;
            active_element = 0;
            buckets.resize(capacity, nullptr);
        }

        c_unordered_map(const c_unordered_map& other)
        {
            capacity = other.capacity;
            active_element = 0;
            buckets.resize(capacity, nullptr);

            for(size_t i = 0; i < capacity; i++)
            {
                HashNode<K,V>* current = other.buckets[i];
                while(current != nullptr)
                {
                    insert(current->key, current->value);
                    current = current->next;
                }
            }
        }

        c_unordered_map(c_unordered_map&& other) noexcept
        {
            capacity = other.capacity;
            active_element = other.active_element;
            buckets = std::move(other.buckets);
            other.capacity = 16;
            other.active_element = 0;
            other.buckets.resize(16, nullptr);
        }

        ~c_unordered_map()
        {
            for(size_t i = 0; i < capacity; i++)
            {
                HashNode<K,V>* current = buckets[i];

                while(current != nullptr)
                {
                    HashNode<K,V>* next_node = current->next;
                    delete current;
                    current = next_node;
                }
            }
        }

        void insert(const K& key, const V& value)
        {
            size_t index = hasher(key) % capacity;

            HashNode<K,V>* current = buckets[index];

            while(current != nullptr)
            {
                if(current->key == key)
                {
                    current->value = value;
                    return;
                }
                current = current->next;
            }

            HashNode<K,V>* new_node = new HashNode<K,V>(key, value, buckets[index]);
            buckets[index] = new_node;
            active_element++;

            check_and_grow();

        }

        std::optional<V> get(const K& key) const
        {
            size_t index = hasher(key) % capacity;
            HashNode<K,V>* current = buckets[index];


            while(current != nullptr)
            {
                if(current->key == key)
                {
                    return current->value;
                }
                current = current->next;
            }
            
            return std::nullopt;
        }

        bool erase(const K& key)
        {
            size_t index = hasher(key) % capacity;
            HashNode<K,V>* current = buckets[index];
            HashNode<K,V>* prev = nullptr;

            while(current != nullptr)
            {
                if(current->key == key)
                {
                    if(prev == nullptr)
                    {
                        buckets[index] = current->next;
                    }
                    else {
                        prev->next = current->next;
                    }

                    delete current;
                    active_element--;
                    return true;
                }

                prev = current;
                current = current->next;
            }
            return false;
        }

        bool empty() const
        {
            return active_element == 0;
        }

        int size() const
        { 
            return active_element;
        }

        void clear()
        {
            for(size_t i = 0; i < capacity; i++)
            {
                HashNode<K,V>* current = buckets[i];

                while(current != nullptr)
                {
                    HashNode<K,V>* next_node = current->next;
                    delete current;
                    current = next_node;
                }
                buckets[i] = nullptr;
            }
            active_element = 0;
        }

        c_unordered_map& operator=( const c_unordered_map& other)
        {
            if(this != &other)
            {
                clear();

                capacity = other.capacity;
                active_element = other.active_element;
                buckets.resize(capacity, nullptr);

                for(size_t i = 0; i < capacity; i++)
                {
                    HashNode<K,V>* current = other.buckets[i];
                    while(current != nullptr)
                    {
                        insert(current->key, current->value);
                        current = current->next;
                    }
                }
            }
            return *this;
        }

        c_unordered_map& operator=(c_unordered_map&& other) noexcept
        {
            if(this != &other)
            {
                clear();

                capacity = other.capacity;
                active_element = other.active_element;
                buckets = std::move(other.buckets);

                other.capacity = 16;
                other.active_element = 0;
                other.buckets.resize(16, nullptr);
            }
            return *this;
        }

        V& operator[](const K& key)
        {
            size_t index = hasher(key) % capacity;
            
            HashNode<K,V>* current = buckets[index];
            while(current != nullptr)
            {
                if(current->key == key) return current->value;
                current = current->next;
            }
            insert(key, V{});
            
            return (*this)[key];
        }

        bool operator==(const c_unordered_map& other) const 
        {
            if(this->active_element != other.active_element) return false;

            for(size_t i = 0; i < capacity; i++)
            {
                HashNode<K,V>* current = buckets[i];
                while(current != nullptr)
                {
                    std::optional<V> other_val = other.get(current->key);

                    if(!other_val.has_value() || other_val.value() != current->value) return false;

                    current = current->next;
                }
            }
            return true;
        }   

        bool operator!=(const c_unordered_map& other) const
        {
            return !(*this == other);
        }


        struct Iterator
        {
            std::vector<HashNode<K,V>*>& buckets;
            size_t bucket_idx;
            HashNode<K,V>* current_node;
            Iterator(std::vector<HashNode<K,V>*>& b, size_t idx, HashNode<K,V>* node)
            : buckets(b), bucket_idx(idx), current_node(node)
            {
                if(current_node == nullptr){
                    advance();
                }
            }
            std::pair<K,V&> operator*()
            {
                return {current_node->key, current_node->value};
            }
            Iterator& operator++()
            {
                if(current_node != nullptr)
                {
                    current_node = current_node->next;
                }
                
                if(current_node == nullptr)
                {
                    bucket_idx++;
                    advance();
                }
                return *this;
            }
            bool operator!=(const Iterator& other) const
            {
                return current_node != other.current_node;
            }

            bool operator==(const Iterator& other) const
            {
                return current_node == other.current_node;
            }
            private:
                void advance()
                {
                    if(current_node != nullptr) return;
                    while(bucket_idx < buckets.size())
                    {
                        if(buckets[bucket_idx] != nullptr)
                        {
                            current_node = buckets[bucket_idx];
                            return;
                        }
                        bucket_idx++;
                    }
                    current_node = nullptr;
                }
        };

        using iterator = Iterator;

        iterator begin()
        {
            return Iterator(buckets, 0, buckets.empty() ? nullptr : buckets[0]);
        }

        iterator end()
        {
            return Iterator(buckets, buckets.size(), nullptr);
        }

    private:
        void check_and_grow()
        {
            float current_load_factor = static_cast<float>(active_element) / capacity;
            
            if(current_load_factor >= MAX_LOAD_FACTOR)
            {
                std::cout << "\n[SYSTEM] Load factor exceeded! Rehashing the size" << 2*capacity << "..\n";
                rehash(capacity*2);
            }
        }

        void rehash(size_t new_capacity)
        {
            std::vector<HashNode<K,V>*> new_buckets(new_capacity, nullptr);

            for(size_t i = 0; i < capacity; i++)
            {
                HashNode<K,V>* current = buckets[i];

                while(current != nullptr)
                {
                    HashNode<K,V>* next_node = current->next;

                    size_t new_index = hasher(current->key) % new_capacity;

                    current->next = new_buckets[new_index];
                    new_buckets[new_index] = current;

                    current = next_node;
                }
            }

            buckets = std::move(new_buckets);
            capacity = new_capacity;
        }

    private:
        std::vector<HashNode<K,V>*> buckets;

        size_t capacity;
        size_t active_element;
        Hash hasher;

        const float MAX_LOAD_FACTOR = 1.0f;
};