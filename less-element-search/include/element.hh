#ifndef __PRIMITIVE_ELEMENT__
#define __PRIMITIVE_ELEMENT__

#include <stdlib.h>
#include "jerasure.h"

/**
 * @brief Generate primitive element from GF(2^w)
 * @param w The word size of the Galois Field, w = 8, 16, 32
 * For primitive elements that are conjugate to each other,
 * we only consider the smallest one in terms of the corresponding decimal integer.
 */

class primitive_element_generator
{
private:
    int w;
    uint32_t cur_exponent;
    uint32_t cur_element;
    uint32_t end_exponent;

public:
    primitive_element_generator(int w) : w(w), cur_exponent(1), cur_element(2)
    {
        if (w == 8)
        {
            end_exponent = 254;
        }
        else if (w == 16)
        {
            end_exponent = 65534;
        }
        else if (w == 32)
        {
            end_exponent = 4294967294;
        }
        else
        {
            end_exponent = 0;
        }
    }
    ~primitive_element_generator() {}
    uint32_t get_element() { return cur_element; }
    void clear()
    {
        cur_exponent = 1;
        cur_element = 2;
    }
    bool next();
    bool is_minimum_conjugate(uint32_t e);
    int minimum_conjugate(uint32_t e);
};

#endif // #ifndef __PRIMITIVE_ELEMENT__