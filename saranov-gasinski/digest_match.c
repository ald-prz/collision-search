#include "digest_match.h"

unsigned char mask[8] = {1, 3, 7, 15, 31, 63, 127, 255};

int digest_match(unsigned char *one, unsigned char *two, int number_of_bits)
{
    const int size = 20;
    int number_of_bytes = number_of_bits / 8;
    int extra_bits = number_of_bits - 8 * number_of_bytes;
    unsigned char first_byte;
    unsigned char second_byte;

    int i;

    for (i = size - 1; i >= size - number_of_bytes; i--)
        if (one[i] != two[i])
            return 0;

    if (number_of_bits != 0)
    {
        first_byte = one[size - number_of_bytes - 1] & mask[extra_bits - 1];
        second_byte = two[size - number_of_bytes - 1] & mask[extra_bits - 1];

        if (first_byte != second_byte)
            return 0;
    }

    return 1;
}
