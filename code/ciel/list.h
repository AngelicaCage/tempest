/* date = October 5th 2023 3:42 pm */

//#include <stdlib.h>

#ifndef CIEL_BASE_H
#include "base.h"
#endif

#ifndef CIEL_LIST_H
#define CIEL_LIST_H

template <typename t>
struct list {
    public:
    b32 allocated;
    u64 length;
    u64 length_allocated;
    t* data;
    
    list()
    {
        allocated = false;
    }
    
    void allocate(u64 size)
    {
        if(allocated)
        {
            print_warning("Allocate called on already-allocated list");
            return;
        }
        allocated = true;
        length = 0;
        length_allocated = size;
        data = (t *)alloc(sizeof(t) * length_allocated);
    }
    
    void allocate()
    {
        allocate(32);
    }
    
    t operator[](u64 index)
    {
        if(index >= length)
        {
            print_warning("index out of bounds");
            return -1;
        }
        return data[index];
    }
    
    t element_at(u64 index)
    {
        return data[index];
    }
    
    void double_size()
    {
        length_allocated *= 2;
        data = (t *)resize_alloc(data, sizeof(t) * length_allocated);
    }
    
    t *add(t new_element)
    {
        if(length + 1 > length_allocated)
            double_size();
        
        data[length] = new_element;
        length++;
        return &(data[length - 1]);
    }
    
    int remove_at(u64 index)
    {
        if(index >= 0 && index < length)
        {
            for(u64 i = index; i < length; i++)
            {
                data[i] = data[i+1];
            }
            length--;
            return 1;
        }
        return 0;
    }
    
    int insert(u64 index, t new_element)
    {
        if(index < 0 || index > length)
        {
            return 1;
        }
        
        if(length + 1 > length_allocated)
        {
            double_size();
        }
        
        for(u64 i = length; i > index; i--)
        {
            data[i] = data[i-1];
        }
        
        data[index] = new_element;
        length++;
        
        return 0;
    }
    
    void del()
    {
        dealloc(data);
        length = 0;
        length_allocated = 0;
        allocated = false;
    }
    
};

template <typename t>
list<t> create_list()
{
    list<t> res;
    res.allocate();
    return res;
}

#endif //CIEL_LIST_H
