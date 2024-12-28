/* date = October 5th 2023 3:42 pm */

//#include <stdlib.h>

#ifndef CIEL_BASE_H
#include "base.h"
#endif

#ifndef CIEL_LIST_H
#define CIEL_LIST_H

template <typename t>
struct List {
    public:
    
    Bool allocated = false;
    U64 length;
    U64 length_allocated;
    t* data;
    
    void allocate(U64 size)
    {
        if(allocated)
        {
            print_warning("Allocate called on already-allocated list");
            return;
        }
        allocated = true;
        length = 0;
        length_allocated = size;
        data = (t *)mem_alloc(sizeof(t) * length_allocated);
    }
    
    void allocate()
    {
        allocate(32);
    }
    
    t operator[](U64 index)
    {
        if(index >= length)
        {
            print_warning("index out of bounds");
            return {0};
        }
        return data[index];
    }
    
    t element_at(U64 index)
    {
        return data[index];
    }
    
    void double_size()
    {
        length_allocated *= 2;
        data = (t *)mem_resize(data, sizeof(t) * length_allocated);
    }
    
    t *add(t new_element)
    {
        if(length + 1 > length_allocated)
            double_size();
        
        data[length] = new_element;
        length++;
        return &(data[length - 1]);
    }
    
    int remove_at(U64 index)
    {
        if(index >= 0 && index < length)
        {
            for(U64 i = index; i < length; i++)
            {
                data[i] = data[i+1];
            }
            length--;
            return 1;
        }
        return 0;
    }
    
    int insert(U64 index, t new_element)
    {
        if(index < 0 || index > length)
        {
            return 1;
        }
        
        if(length + 1 > length_allocated)
        {
            double_size();
        }
        
        for(U64 i = length; i > index; i--)
        {
            data[i] = data[i-1];
        }
        
        data[index] = new_element;
        length++;
        
        return 0;
    }
};

template <typename t>
List<t> allocate_list()
{
    List<t> res;
    res.allocated = false;
    res.allocate();
    return res;
}

template <typename T, Int length_allocated>
struct InplaceList
{
    T data[length_allocated];
    Int length = 0;
    
    inline T *add(T element) {
        if(length < length_allocated - 1) {
            data[length] = element;
            length++;
        }
        return &(data[length-1]);
    }
    inline Void pop() {
        length--;
    }
    inline int remove_at(U64 index)
    {
        if(index >= 0 && index < length)
        {
            for(U64 i = index; i < length; i++)
            {
                data[i] = data[i+1];
            }
            length--;
            return 1;
        }
        return 0;
    }
    inline T& operator[](Int index) {
        return data[index];
    }
};

template <typename T, Int length_allocated>
struct InplaceStack
{
    T data[length_allocated];
    Int length = 0;
    
    inline Void push(T element) {
        if(length < length_allocated - 1) {
            data[length] = element;
            length++;
        }
    }
    inline Void pop() {
        length--;
    }
    inline T& operator[](Int index) {
        return data[index];
    }
};

template <typename T, Int length_allocated>
struct InplaceCircularArray
{
    T data[length_allocated];
    Int length = 0;
    Int start;
    
    Void add(T element)
    {
        if(length < length_allocated) {
            data[length] = element;
            length++;
        }
        else
        {
            data[start] = element;
            start++;
            if(start >= length_allocated)
                start = 0;
        }
    }
    T& element_at(Int index)
    {
        if(length == length_allocated)
        {
            Int adjusted_index = (start + index) % length_allocated;
            return data[adjusted_index];
        }
        if(index < 0)
            index = 0;
        return data[index];
    }
    T& operator[](Int index)
    {
        return element_at(index);
    }
    T& last()
    {
        // TODO: is this right?
        return element_at(length-1);
    }
};


#endif //CIEL_LIST_H
