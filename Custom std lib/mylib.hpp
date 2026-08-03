#pragma once
#include <cstddef>
#include<iostream>

namespace mylib
{
    template< typename T>
    class Vector
    {
        public:
            Vector()
            {
                v_capacity = 4;
                v_size = 0;
                Data = new T[v_capacity];
            }

            Vector(size_t size, T value)
            {
                v_capacity = size;
                v_size = size;
                Data = new T[size];
                for(size_t i = 0; i < size; i++)
                {
                    Data[i] = value;
                }
            }

            ~Vector()
            {
                delete[] Data;
            }

            void resize(size_t size)
            {
                if(size < v_size) return;

                T* temp = new T[size];
                for(size_t i = 0; i < v_size; i++)
                {
                    temp[i] = Data[i];
                }
                
                delete[] Data;

                Data = temp;
                v_capacity = size;
            }

            void push_back(T data)
            {   
                if(v_size >= v_capacity) resize(v_capacity*2);

                Data[v_size] = data;
                v_size++;
            }

            // vec.pop_back() remove last element
            void pop_back()
            {
                if(v_size == 0) return;
                v_size--;

                if(v_size > 0 && v_size == (v_capacity / 4) && v_capacity > 4)
                {
                    v_capacity /= 2;
                    T* temp = new T[v_capacity];
                    
                    for(size_t i = 0; i < v_size; i++)
                    {
                        temp[i] = Data[i];
                    }

                    delete[] Data;

                    Data = temp;
                } 
            }

            // vec.clear to clear the whole vector
            void clear() 
            {
                v_size = 0;
            }

            size_t size()
            {
                return v_size;
            }


            // vec.at()
            T& at(size_t index)
            {
                if(index >= v_size) throw std::out_of_range("Index out of vector size!!");
                return Data[index];
            }


            T& operator[](size_t index)
            {
                return Data[index];
            }


            // Copy assignment operator
            Vector operator=(const Vector& other) {
                if(this != &other)
                {
                    delete[] Data;
                    
                    v_size = other.v_size;
                    v_capacity = other.v_capacity;
                    Data = new T[v_capacity];

                    for(size_t i = 0; i < v_size; i++)
                    {
                        Data[i] = other.Data[i]; 
                    }
                }

                return *this;
            }


            // Copy constructor
            Vector(const Vector&other)
            {
                v_size = other.v_size;
                v_capacity = other.v_capacity;
                
                Data = new T[v_size];
                
                for(size_t i = 0; i < v_size; i++)
                {
                    Data[i] = other.Data[i];
                }   
            }
        
        private:
            T* Data;
            size_t v_size;
            size_t v_capacity;

    };


    class String
    {
        public:    

            class Iterator 
            {
                public:
                    Iterator(char* ptr)
                    {
                        m_ptr = ptr;
                    }
                    char& operator*()
                    {
                        return *m_ptr;
                    }

                    Iterator& operator++()
                    {
                        m_ptr++;
                        return *this;
                    }

                    bool operator!=(const Iterator& other)
                    {
                        return m_ptr != other.m_ptr;
                    }
                
                private:
                    char* m_ptr;

            };

            Iterator begin()
            {
                return Iterator(Data); 
            }

            Iterator end()
            {
                return Iterator(Data + s_size);
            }

            String()
            {
                s_size = 0;
                s_capacity = 4;
                Data = new char[1];
                Data[0] = '\0';
            }

            String(const char* string)
            {
                s_size = 0;  
                while(string[s_size] != '\0') s_size++;
                
                s_capacity = s_size + 4;
                Data = new char[s_capacity];

                for(size_t i = 0; i < s_size; i++)
                {
                    Data[i] = string[i];
                }
                Data[s_size]  = '\0';
            }

            String(const String& other)
            {
                s_size = other.s_size;
                s_capacity = other.s_capacity;
                
                Data = new char[s_capacity];
                
                for(size_t i = 0; i < s_size; i++)
                {
                    Data[i] = other.Data[i];
                }
                Data[s_size] = '\0';
            }

            String operator+(const String& str2) const
            {
                String result = *this; 
                result += str2;        
                return result;
            }

            ~String()
            {
                delete[] Data;
            }

            void resize(size_t size)
            {
                if(s_capacity > size) throw std::length_error("Size less than capacity!!");

                s_capacity = size;
                char* temp = new char[size];

                for(size_t i=0; i<s_size; i++)
                {
                    temp[i] = Data[i];
                }
                temp[s_size] = '\0';
                delete[] Data;
                Data = temp;
            }

            size_t Begin()
            {
                return 0;
            }

            size_t End()
            {
                return s_size;
            }

            int size()
            {
                return (int)s_size;
            }

            void clear()
            {
                delete[] Data;
                s_size = 0;
                s_capacity = 4;
                
                Data = new char[1];
                Data[0] = '\0';
            }

            const char& back()
            {   
                return Data[s_size-1];
            }

            const char& front()
            {
                return Data[0];
            }

            void push_back(char c)
            {
                if(s_size >= s_capacity) resize(s_capacity * 2);
                Data[s_size] = c;
                Data[s_size+1] = '\0';
                s_size++;
            }

            void pop_back()
            {
                if(s_size == 0) return;
                s_size--;
                if(s_size > 0 && s_size == (s_capacity/4) && s_capacity >0)
                {
                    s_capacity /= 2;
                    char* temp = new char[s_capacity];

                    for(size_t i =0; i < s_size; i++)
                    {
                        temp[i] = Data[i];
                    }

                    temp[s_size] = '\0';
                    delete[] Data;

                    Data = temp;
                }
            }

            String operator+=(const String& str2)
            {
                size_t old_size = s_size;

                s_size += str2.s_size;

                if(s_size >= s_capacity) resize(s_size + 4);

                for (size_t i =0 ;i < str2.s_size; i++)
                {
                    Data[old_size + i]  = str2.Data[i];
                }

                Data[s_size] = '\0';

                return *this;
            }

            String& operator=(const String& other)
            {
                if(this != &other)
                {
                    delete[] Data;

                    s_size = other.s_size;
                    s_capacity = other.s_capacity;
                    
                    Data  = new char[s_capacity];

                    for(size_t i = 0; i < s_size; i++)
                    {
                        Data[i] = other.Data[i];
                    }
                    Data[s_size] = '\0';
                }

                return *this;
            }

            char& operator[](size_t index)
            {
                return Data[index];
            }

            bool operator== (const String& str)
            {
                if(s_size != str.s_size) return false;
                            
                for(size_t i = 0; i < s_size; i++) 
                {
                    if(Data[i] != str.Data[i]) return false;
                }
                return true;
            }

            void print()
            {
                std::cout<< Data << std::endl;
            }

        private:
            char* Data;
            size_t s_size;
            size_t s_capacity;
        };
}