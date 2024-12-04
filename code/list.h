/* date = October 5th 2023 3:42 pm */

#ifndef CIEL_BASE_H
#include "base.h"
#endif

#ifndef CIEL_LIST_H
#define CIEL_LIST_H

template <typename t>
struct List {
    public:
    
    Bool allocated;
    U64 length;
    U64 length_allocated;
    t* data;
    
    List()
    {
        allocated = false;
    }
    
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
        data = (t *)alloc(sizeof(t) * length_allocated);
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
    
    void free()
    {
        free(data);
        length = 0;
        length_allocated = 0;
        allocated = false;
    }
    
};

template <typename t>
List<t> create_list()
{
    List<t> res;
    res.allocated = false;
    res.allocate();
    return res;
}

#endif //CIEL_LIST_H
