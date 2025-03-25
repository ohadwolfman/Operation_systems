#include "pythagorean.h"

int is_pythagorean_triple(unsigned char a, unsigned char b, unsigned char c) {
    unsigned char x, y, z;

    // Sorting the numbers such that z is the largest (for C)
    if (a >= b && a >= c) { z = a; x = b; y = c; }
    else if (b >= a && b >= c) { z = b; x = a; y = c; }
    else { z = c; x = a; y = b; }

    return (x*x + y*y == z*z);
}
