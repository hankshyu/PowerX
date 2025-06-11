#include <iostream>
#include <algorithm>


class Foo{

    private:
        int *array;
        int size;
    
    public:
        // constructor
        Foo (int s): size(s), array(new int[s]{0}) {
    
        }
    
        // destructor
        ~Foo() {
            delete[] array;
        }
    
        // copy constructor
        Foo(const Foo &other): size(other.size), array(new int[other.size]) {
            for (int i = 0; i < size; ++i){
                array[i] = other.array[i];
            }
        }
    
        // move constructor
        Foo(Foo && other) noexcept : size(0), array(nullptr) {
            swap(*this, other);
        }
    
        // copy assignment operator using copy and swap
        Foo &operator= (Foo other){
            swap(*this, other);
            return *this;
        }
        
        // move assignment operator 
        Foo &operator= (Foo &&other) noexcept{
            if(this != &other){
    
                delete [] this->array;
    
                this->array = other.array;
                this->size = other.size;
    
                other.array = nullptr;
                other.size = 0;
            }
    
            return *this;
        }

        // swap function
        friend void swap (Foo &first, Foo &second) noexcept {
            std::swap(first.size, second.size);
            std::swap(first.array, second.array);
        }
    
};